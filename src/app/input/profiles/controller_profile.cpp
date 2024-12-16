#include "controller_profile.hpp"

namespace firelight::input {
    ControllerProfile::ControllerProfile(const int vendorId, const int productId,
                                         const int productVersion) : m_vendorId(vendorId),
                                                                     m_productId(productId),
                                                                     m_productVersion(productVersion) {
        static int i = 0;
        m_id = i++;
    }

    int ControllerProfile::getId() const {
        return m_id;
    }

    int ControllerProfile::getVendorId() const {
        return m_vendorId;
    }

    int ControllerProfile::getProductId() const {
        return m_productId;
    }

    int ControllerProfile::getProductVersion() const {
        return m_productVersion;
    }
}
