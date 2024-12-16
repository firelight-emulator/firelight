#pragma once

namespace firelight::input {
    class ControllerProfile {
    public:
        ControllerProfile(int vendorId, int productId, int productVersion);

        int getId() const;

        int getVendorId() const;

        int getProductId() const;

        int getProductVersion() const;

    private:
        int m_id = 0;
        int m_vendorId = 0;
        int m_productId = 0;
        int m_productVersion = 0;
    };
}
