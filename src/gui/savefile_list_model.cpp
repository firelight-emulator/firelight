#include "savefile_list_model.hpp"

namespace firelight::gui {

SavefileListModel::SavefileListModel(db::IUserdataDatabase &userdataDatabase)
    : m_userdataDatabase(userdataDatabase) {}
int SavefileListModel::rowCount(const QModelIndex &parent) const {}
QVariant SavefileListModel::data(const QModelIndex &index, int role) const {}

} // namespace firelight::gui
  // firelight