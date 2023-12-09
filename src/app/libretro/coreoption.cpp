//
// Created by alexs on 10/14/2023.
//

// #include <nlohmann/json.hpp>
#include "coreoption.hpp"

// using nlohmann::json;

namespace libretro {
CoreOption::CoreOption(retro_core_option_definition definition) {
  this->key = definition.key;
  this->defaultValue = definition.default_value;
  this->description = definition.desc;
  this->info = definition.info;
  this->defaultValueIndex = 0;
  this->currentValue = this->defaultValue;

  for (auto value : definition.values) {
    if (definition.key == nullptr) {
      break;
    }

    this->values.push_back(value);
  }
}

CoreOption::CoreOption(retro_core_option_v2_definition definition) {
  this->key = definition.key;
  this->defaultValue = definition.default_value;
  this->descCategorized = definition.desc_categorized;
  this->description = definition.desc;
  this->info = definition.info;
  this->infoCategorized = definition.info_categorized;
  this->categoryKey = definition.category_key;
  this->defaultValueIndex = 0;
  this->currentValue = this->defaultValue;

  for (auto value : definition.values) {
    if (definition.key == nullptr) {
      break;
    }

    this->values.push_back(value);
  }
}

//    json CoreOption::dumpJson() {
//        json opt;
//
//        if (this->key != nullptr) {
//            opt["key"] = this->key;
//        }
//
//        if (this->currentValue != nullptr) {
//            opt["currentValue"] = this->currentValue;
//        }
//
//        if (this->description != nullptr) {
//            opt["description"] = this->description;
//        }
//        if (this->info != nullptr) {
//            opt["info"] = this->info;
//        }
//        if (this->defaultValue != nullptr) {
//            opt["defaultValue"] = this->defaultValue;
//        }
//        if (this->infoCategorized != nullptr) {
//            opt["infoCategorized"] = this->infoCategorized;
//        }
//        if (this->categoryKey != nullptr) {
//            opt["categoryKey"] = this->categoryKey;
//        }
//
//        opt["visible"] = this->displayToUser;
//
//        return opt;
//    }
} // namespace libretro