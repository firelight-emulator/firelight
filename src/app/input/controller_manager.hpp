#pragma once

#include "firelight/libretro/retropad_provider.hpp"
#include "sdl_controller.hpp"
#include <QAbstractListModel>
#include <QObject>
#include <SDL_events.h>

#include "controller_repository.hpp"
#include "keyboard_input_handler.hpp"

#include <qsettings.h>

namespace firelight::input {
class ControllerManager final : public QObject,
                                public libretro::IRetropadProvider {
  Q_OBJECT
  Q_PROPERTY(bool blockGamepadInput READ blockGamepadInput WRITE
                 setBlockGamepadInput NOTIFY blockGamepadInputChanged)
  Q_PROPERTY(bool prioritizeControllerOverKeyboard READ
                 prioritizeControllerOverKeyboard WRITE
                     setPrioritizeControllerOverKeyboard NOTIFY
                         prioritizeControllerOverKeyboardChanged)
  Q_PROPERTY(
      bool onlyPlayerOneCanNavigateMenus READ getOnlyPlayerOneCanNavigateMenus
          WRITE setOnlyPlayerOneCanNavigateMenus NOTIFY
              onlyPlayerOneCanNavigateMenusChanged)

public:
  bool m_blockGamepadInput = false;

  explicit ControllerManager(IControllerRepository &controllerRepository);

  void setKeyboardRetropad(KeyboardInputHandler *keyboard);

  void handleSDLControllerEvent(const SDL_Event &event);

  int getPlayerIndexForControllerButtonEvent(
      const SDL_ControllerButtonEvent &event) const;

  void refreshControllerList();

  [[nodiscard]] std::optional<IGamepad *>
  getControllerForPlayerIndex(int t_player) const;

  std::optional<libretro::IRetroPad *>
  getRetropadForPlayerIndex(int t_player, int platformId) override;

  Q_INVOKABLE void updateControllerOrder(const QVector<int> &order);

  bool blockGamepadInput() const;

  void setBlockGamepadInput(bool blockGamepadInput);

  void
  setPrioritizeControllerOverKeyboard(bool prioritizeControllerOverKeyboard);

  [[nodiscard]] bool prioritizeControllerOverKeyboard() const;

  void setOnlyPlayerOneCanNavigateMenus(bool onlyPlayerOneCanNavigateMenus);

  [[nodiscard]] bool getOnlyPlayerOneCanNavigateMenus() const;

public slots:
  void updateControllerOrder(const QVariantMap &map);

signals:
  void controllerConnected(int playerNumber, QString controllerName,
                           QString iconSourceUrl);

  void controllerDisconnected(int playerNumber, QString controllerName,
                              QString iconSourceUrl);

  void controllerOrderChanged();

  void prioritizeControllerOverKeyboardChanged();

  void retropadInputStateChanged(int playerNumber, int input, bool activated);

  void buttonStateChanged(int playerNumber, int button, bool pressed);

  void axisStateChanged(int playerNumber, int axis, int value);

  void blockGamepadInputChanged();

  void onlyPlayerOneCanNavigateMenusChanged();

private:
  std::unique_ptr<QSettings> m_settings;
  IControllerRepository &m_controllerRepository;
  int m_numControllers = 0;
  std::array<std::unique_ptr<SdlController>, 32> m_controllers{};
  bool m_prioritizeControllerOverKeyboard;
  bool m_onlyPlayerOneCanNavigateMenus = true;

  void openControllerWithDeviceIndex(int32_t t_deviceIndex);
};
} // namespace firelight::input
