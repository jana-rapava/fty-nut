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

#ifndef NUT_AGENT_H_INCLUDED
#define NUT_AGENT_H_INCLUDED

#define NUT_INVENTORY_REPEAT_AFTER      3600000

class NUTAgent {
 public:
    bool loadMapping (const char *path_to_file);
    bool isMappingLoaded ();

    void setClient (mlm_client_t *client);
    void setiClient (mlm_client_t *client);
    bool isClientSet ();

    void onPoll ();
    void TTL (int ttl) { _ttl = ttl; };
    int TTL () { return _ttl; };
 protected:
    std::string physicalQuantityShortName (const std::string& longName);
    std::string physicalQuantityToUnits (const std::string& quantity);
    void advertisePhysics ();
    void advertiseInventory ();
    int send (const std::string& subject, zmsg_t **message_p);
    int isend (const std::string& subject, zmsg_t **message_p);

    int _ttl = 60;
        
    drivers::nut::NUTDeviceList _deviceList;
    uint64_t _inventoryTimestamp = 0;

    static const std::map <std::string, std::string> _units;

    std::string _conf;
    mlm_client_t *_client = NULL;
    mlm_client_t *_iclient = NULL;
};

//  Self test of this class
AGENT_NUT_EXPORT void
    nut_agent_test (bool verbose);
//  @end

#endif
