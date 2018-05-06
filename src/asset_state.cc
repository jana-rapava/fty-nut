/*  =========================================================================
    asset_state - list of known assets

    Copyright (C) 2014 - 2017 Eaton

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
    asset_state - list of known assets
@discuss
@end
*/

#include "asset_state.h"
#include "logger.h"

#include <cmath>

AssetState::Asset::Asset(fty_proto_t* message)
{
    name_ = fty_proto_name(message);
    IP_ = fty_proto_ext_string(message, "ip.1", "");
    port_ = fty_proto_ext_string(message, "port", "");
    subtype_ = fty_proto_aux_string(message, "subtype", "");
    location_ = fty_proto_aux_string(message, "parent_name.1", "");
    max_current_ = NAN;
    try {
        max_current_ = std::stod(fty_proto_ext_string(message,
                    "max_current", ""));
    } catch (...) { }
    max_power_ = NAN;
    try {
        max_power_ = std::stod(fty_proto_ext_string(message, "max_power", ""));
    } catch (...) { }
    daisychain_ = 0;
    try {
        daisychain_ = std::stoi(fty_proto_ext_string(message,
                    "daisy_chain", ""));
    } catch (...) { }
}

void AssetState::updateFromProto(fty_proto_t* message)
{
    std::string type(fty_proto_aux_string (message, "type", ""));

    if (type != "device")
        return;
    std::string subtype(fty_proto_aux_string (message, "subtype", ""));
    AssetMap* map;
    if (subtype == "epdu" || subtype == "ups" || subtype == "sts")
        map = &powerdevices_;
    else if (subtype == "sensor" || subtype == "sensorgpio")
        map = &sensors_;
    else
        return;
    std::string name(fty_proto_name(message));
    std::string operation(fty_proto_operation(message));
    if (operation == FTY_PROTO_ASSET_OP_DELETE ||
            operation == FTY_PROTO_ASSET_OP_RETIRE) {
        map->erase(name);
        return;
    }
    if (operation != FTY_PROTO_ASSET_OP_CREATE &&
            operation != FTY_PROTO_ASSET_OP_UPDATE) {
        log_error("unknown asset operation '%s'. Skipping.",
                operation.c_str());
        return;
    }
    (*map)[name] = std::shared_ptr<Asset>(new Asset(message));
}

void AssetState::recompute()
{
    ip2master_.clear();
    for (auto i : powerdevices_) {
        const std::string& ip = i.second->IP();
        if (ip == "") {
            // this is strange. No IP?
            continue;
        }
        if (i.second->daisychain() <= 1) {
            // this is master
            ip2master_[ip] = i.first;
        }
    }
}

const std::string& AssetState::ip2master(const std::string& ip) const
{
    static const std::string empty;

    const auto i = ip2master_.find(ip);
    if (i == ip2master_.cend())
        return empty;
    return i->second;
}
