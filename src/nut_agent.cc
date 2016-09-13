/*  =========================================================================
    nut_agent - NUT daemon wrapper

    Copyright (C) 2014 - 2015 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

/*
@header
    nut_agent - NUT daemon wrapper
@discuss
@end
*/
#include <cmath>

#include "agent_nut_classes.h"

const std::map<std::string, std::string> NUTAgent::_units =
{
    { "temperature", "C" },
    { "realpower",   "W" },
    { "voltage",     "V" },
    { "current",     "A" },
    { "load",        "%" },
    { "charge",      "%" },
    { "frequency",   "Hz"},
    { "power",       "W" },
    { "runtime",     "s" },
};

bool NUTAgent::loadMapping (const char *path_to_file)
{
    _conf = path_to_file;
    _deviceList.load_mapping (_conf.c_str ());
    return _deviceList.mappingLoaded ();
}

bool NUTAgent::isMappingLoaded () const
{
    return _deviceList.mappingLoaded ();
}

void NUTAgent::setClient (mlm_client_t *client)
{
    if (!_client) {
       _client = client;
    }
}
void NUTAgent::setiClient (mlm_client_t *client)
{
    if (!_iclient) {
       _iclient = client;
    }
}

bool NUTAgent::isClientSet () const
{
    return _client != NULL;
}

void NUTAgent::onPoll ()
{
    if (_client)
        advertisePhysics ();
    if (_iclient)
        advertiseInventory ();
}

void NUTAgent::updateDeviceList (nut_t *deviceState) {
    _deviceList.updateDeviceList (deviceState);
}

int NUTAgent::send (const std::string& subject, zmsg_t **message_p)
{
    bios_proto_t *m_decoded = bios_proto_decode(message_p);
    zmsg_destroy(message_p);
    bios_proto_aux_insert(m_decoded, "time", "%" PRIi64, zclock_time () / 1000);
    *message_p = bios_proto_encode(&m_decoded);

    int rv = mlm_client_send (_client, subject.c_str (), message_p);
    if (rv == -1) {
        log_error ("mlm_client_send (subject = '%s') failed", subject.c_str ());
    }
    zmsg_destroy (message_p);
    return rv;
}

//MVY: a hack for inventory messages
int NUTAgent::isend (const std::string& subject, zmsg_t **message_p)
{
    bios_proto_t *m_decoded = bios_proto_decode(message_p);
    zmsg_destroy(message_p);
    bios_proto_aux_insert(m_decoded, "time", "%" PRIi64, zclock_time () / 1000);
    *message_p = bios_proto_encode(&m_decoded);

    int rv = mlm_client_send (_iclient, subject.c_str (), message_p);
    if (rv == -1) {
        log_error ("mlm_client_send (subject = '%s') failed", subject.c_str ());
    }
    zmsg_destroy (message_p);
    return rv;
}

std::string NUTAgent::physicalQuantityShortName (const std::string& longName)
{
    size_t i = longName.find ('.');
    if (i == std::string::npos) {
       return longName;
    }
    return longName.substr (0, i);
}

std::string NUTAgent::physicalQuantityToUnits (const std::string& quantity) {
    auto it = _units.find(quantity);
    if (it == _units.end ()) {
        return "";
    }
    return it->second;
}

void NUTAgent::advertisePhysics () {

    _deviceList.update (true);
    for (auto& device : _deviceList) {
        std::string subject;
        for (auto& measurement : device.second.physics (false)) {
            subject = measurement.first + "@" + device.second.assetName ();
            std::string type = physicalQuantityShortName (measurement.first);
            std::string units = physicalQuantityToUnits (type);
            if (units.empty ()) {
                log_error ("undefined physical quantity '%s'", type.c_str ());
                continue;
            }

            double d_value = measurement.second * std::pow (10, -2);
            char buffer [50];
            sprintf (buffer, "%lf", d_value);

            zmsg_t *msg = bios_proto_encode_metric (
                NULL,
                measurement.first.c_str (),
                device.second.assetName ().c_str (),
                buffer,
                units.c_str (),
                _ttl);
            if (msg) {
                log_debug ("sending new measurement for element_src = '%s', type = '%s', value = '%s', units = '%s'",
                           device.second.assetName ().c_str (), measurement.first.c_str (), buffer, units.c_str ());

                int r = send(subject, &msg);
                if( r != 0 ) log_error("failed to send measurement %s result %" PRIi32, subject.c_str(), r);
                zmsg_destroy (&msg);
                device.second.setChanged (measurement.first, false);
            }
        }
        // send also status as bitmap
        if (device.second.hasProperty ("status.ups")) {
            subject = "status@" + device.second.assetName ();
            std::string status_s = device.second.property ("status.ups");
            uint16_t    status_i = upsstatus_to_int (status_s);
            zmsg_t *msg = bios_proto_encode_metric (
                NULL,
                "status.ups",
                device.second.assetName ().c_str (),
                std::to_string (status_i).c_str (),
                "",
                _ttl);
            if (msg) {
                log_debug ("sending new status for element_src = '%s', value = '%s' (%s)",
                           device.second.assetName().c_str (), std::to_string (status_i).c_str (), status_s.c_str ());
                int r = send (subject, &msg);
                if( r != 0 ) log_error("failed to send measurement %s result %" PRIi32, subject.c_str(), r);
                zmsg_destroy (&msg);
                device.second.setChanged ("status.ups", false);
            }
        }
        //MVY: send also epdu status as bitmap
        for (int i = 1; i != 100; i++) {
            std::string property = "status.outlet." + std::to_string (i);
            // assumption, if outlet.10 does not exists, outlet.11 does not as well
            if (!device.second.hasProperty (property))
                break;
            subject = "status.outlet." + std::to_string (i) + "@" + device.second.assetName ();
            std::string status_s = device.second.property (property);
            uint16_t    status_i = status_s == "on" ? 42 : 0;

            zmsg_t *msg = bios_proto_encode_metric (
                NULL,
                property.c_str (),
                device.second.assetName ().c_str (),
                std::to_string (status_i).c_str (),
                "",
                _ttl);
            if (msg) {
                log_debug ("sending new status for %s %s, value %i (%s)",
                           property.c_str (),
                           device.second.assetName().c_str(),
                           status_i,
                           status_s.c_str());
                int r = send (subject, &msg);
                if( r != 0 ) log_error("failed to send measurement %s result %" PRIi32, subject.c_str(), r);
                zmsg_destroy (&msg);
                device.second.setChanged (property, false);
            }
        }
    }
}

void NUTAgent::advertiseInventory() {
    bool advertiseAll = false;
    if (_inventoryTimestamp_ms + NUT_INVENTORY_REPEAT_AFTER_MS < static_cast<uint64_t> (zclock_mono ())) {
        advertiseAll = true;
        _inventoryTimestamp_ms = static_cast<uint64_t> (zclock_mono ());
    }
    for (auto& device : _deviceList) {
        std::string log;
        zhash_t *inventory = zhash_new ();
        // !advertiseAll = advetise_Not_OnlyChanged
        for (auto& item : device.second.inventory (!advertiseAll) ) {
            if (item.first == "status.ups") {
                // this value is not advertised as inventory information
                continue;
            }
            zhash_insert (inventory, item.first.c_str (), (void *) item.second.c_str ()) ;
            log += item.first + " = \"" + item.second + "\"; ";
            device.second.setChanged (item.first, false);
        }
        if (zhash_size (inventory) == 0) {
            zhash_destroy (&inventory);
            continue;
        }

        zmsg_t *message = bios_proto_encode_asset (
                NULL,
                device.second.assetName().c_str(),
                "inventory",
                inventory);

        if (message) {
            std::string topic = "inventory@" + device.second.assetName();
            log_debug ("new inventory message '%s': %s", topic.c_str(), log.c_str());
            int r = isend (topic, &message);
            if( r != 0 )
                log_error ("failed to send inventory %s result %i", topic.c_str(), r);
            zmsg_destroy (&message);
        }
        zhash_destroy (&inventory);
    }
}

//  --------------------------------------------------------------------------
//  Self test of this class

void
nut_agent_test (bool verbose)
{
    printf (" * nut_agent: ");

    //  @selftest
    //  @end
    printf ("OK\n");
}
