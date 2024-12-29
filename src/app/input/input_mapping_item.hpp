#pragma once

#include <QObject>
#include <QVariantMap>
#include <firelight/libretro/retropad.hpp>

#include "../manager_accessor.hpp"

namespace firelight::input {
    class InputMappingItem : public QObject, public ManagerAccessor {
        Q_OBJECT
        Q_PROPERTY(int profileId READ getProfileId WRITE setProfileId NOTIFY profileIdChanged)
        Q_PROPERTY(int platformId READ getPlatformId WRITE setPlatformId NOTIFY platformIdChanged)
        Q_PROPERTY(QVariantMap inputMappings READ getInputMappings NOTIFY inputMappingsChanged)

    public:
        explicit InputMappingItem(QObject *parent = nullptr);

        [[nodiscard]] QVariantMap getInputMappings() const;

        int getProfileId() const;

        int getPlatformId() const;

        void setProfileId(int profileId);

        void setPlatformId(int platformId);

        Q_INVOKABLE void addMapping(int input, int mappedInput);

        Q_INVOKABLE void removeMapping(int input);

    signals:
        void inputMappingsChanged();

        void profileIdChanged();

        void platformIdChanged();

    private:
        std::shared_ptr<InputMapping> m_inputMapping;
        int m_profileId = 0;
        int m_platformId = 0;

        void refreshMapping();

        QVariantMap m_inputMappings{};
    };
} // input
// firelight
