#pragma once

#include <string>

namespace firelight::input {
    class ControllerProfile {
    public:
        ControllerProfile(int id, const std::string &name, int vendorId, int productId, int productVersion);

        int getId() const;

        int getVendorId() const;

        int getProductId() const;

        int getProductVersion() const;

        std::string getDisplayName() const;

    private:
        std::string m_displayName;
        int m_id = 0;
        int m_vendorId = 0;
        int m_productId = 0;
        int m_productVersion = 0;
    };
}
