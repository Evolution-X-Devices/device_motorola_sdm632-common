//
// SPDX-FileCopyrightText: The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
//

#include <sys/sysinfo.h>
#include <android-base/logging.h>
#include <android-base/properties.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

using android::base::GetProperty;
using android::base::SetProperty;

constexpr const char* RO_PROP_SOURCES[] = {
    nullptr,
    "bootimage.",
    "odm.",
    "odm_dlkm.",
    "product.",
    "system.",
    "system_dlkm.",
    "system_ext.",
    "vendor.",
    "vendor_dlkm.",
};

/*
 * SetProperty does not allow updating read only properties and as a result
 * does not work for our use case. Write "OverrideProperty" to do practically
 * the same thing as "SetProperty" without this restriction.
 */
void OverrideProperty(const char* name, const char* value) {
    size_t valuelen = strlen(value);

    prop_info* pi = (prop_info*)__system_property_find(name);
    if (pi != nullptr) {
        __system_property_update(pi, value, valuelen);
    } else {
        __system_property_add(name, strlen(name), value, valuelen);
    }
}

void OverrideCarrierProperties() {
    const auto ro_prop_override = [](const char* source, const char* prop, const char* value,
                                     bool product) {
        std::string prop_name = "ro.";

        if (product) prop_name += "product.";
        if (source != nullptr) prop_name += source;
        if (!product) prop_name += "build.";
        prop_name += prop;

        OverrideProperty(prop_name.c_str(), value);
    };

    // Setting carrier prop
    std::string carrier = GetProperty("ro.boot.carrier", "unknown");
    OverrideProperty("ro.carrier", carrier.c_str());

    std::string sku = GetProperty("ro.boot.hardware.sku", "");
    if (sku == "XT1952-T") {
        /* T-Mobile REVVLRY */
        for (const auto &source : RO_PROP_SOURCES) {
            ro_prop_override(source, "fingerprint", "motorola/channel_revvl/channel:10/QPY30.85-18/6572f:user/release-keys", true);
            ro_prop_override(source, "model", "REVVLRY", true);
            ro_prop_override(source, "name", "channel_revvl", true);
        }
        ro_prop_override(nullptr, "description", "channel_revvl-user 10 QPY30.85-18 6572f release-keys", false);
        ro_prop_override(nullptr, "product", "channel_revvl", false);
        OverrideProperty("persist.vendor.radio.customer_mbns", "tmo_usa_ims_default.mbn;sprint_usa_ims.mbn");
        OverrideProperty("persist.vendor.radio.data_con_rprt", "1");
        OverrideProperty("persist.vendor.ims.playout_delay", "10");
        OverrideProperty("persist.vendor.ims.cam_sensor_delay", "20");
        OverrideProperty("persist.vendor.ims.display_delay", "40");
    }
}

void vendor_load_properties() {
    OverrideCarrierProperties();
}
