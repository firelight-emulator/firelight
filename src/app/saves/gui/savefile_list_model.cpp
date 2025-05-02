#include "savefile_list_model.hpp"

namespace firelight::gui {

SavefileListModel::SavefileListModel(db::IUserdataDatabase &userdataDatabase)
    : m_userdataDatabase(userdataDatabase) {}
int SavefileListModel::rowCount(const QModelIndex &parent) const { return 0; }
QVariant SavefileListModel::data(const QModelIndex &index, int role) const {return {};}

} // namespace firelight::gui
  // firelight