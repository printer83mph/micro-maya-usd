#include "utils.h"

#include <QMessageBox>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/variantSets.h>

#include <regex>

float utils::getRandom() { return (float)rand() / (float)RAND_MAX; }

glm::vec3 utils::getRandomColor() {
  return glm::vec3(getRandom(), getRandom(), getRandom());
}

bool utils::verifyUsdFile(QWidget *parent,
                          const std::filesystem::path &filePath) {
  if (!std::filesystem::exists(filePath)) {
    QMessageBox::warning(parent, "Invalid USD file",
                         ("File not found: " + filePath.string()).c_str());
    return false;
  }
  std::string fileName = filePath.filename().string();

  auto assetName = fileName.substr(0, fileName.find_last_of('.'));
  if (!std::regex_match(assetName, std::regex(R"(^[a-z]+([A-Z][a-z]+)*$)"))) {
    QMessageBox::warning(parent, "Invalid USD file",
                         "Error: Asset name is not in camelCase");
    return false;
  }

  if (filePath.extension() != ".usda") {
    QMessageBox::warning(parent, "Invalid USD file",
                         "Error: File does not have .usda extension");
    return false;
  }

  auto stage = pxr::UsdStage::Open(filePath);
  if (!stage) {
    QMessageBox::warning(parent, "Invalid USD file",
                         "Error: Failed to open USD stage");
    return false;
  }

  if (!stage->HasDefaultPrim()) {
    QMessageBox::warning(parent, "Invalid USD file",
                         "Error: No default prim found");
    return false;
  }

  auto defaultPrim = stage->GetDefaultPrim();
  pxr::SdfPath defaultPrimPath = defaultPrim.GetPath();
  if (defaultPrimPath.GetParentPath() != pxr::SdfPath::AbsoluteRootPath()) {
    QMessageBox::warning(
        parent, "Invalid USD file",
        "Error: Default prim is not a direct child of the root");
    return false;
  }

  auto defPrimName = defaultPrimPath.GetName();
  if (defPrimName != assetName) {
    QMessageBox::warning(parent, "Invalid USD file",
                         "Error: Default prim name does not match file name");
    return false;
  }

  auto variantSets = defaultPrim.GetVariantSets();
  auto variantSetNames = variantSets.GetNames();
  if (variantSetNames.size() != 1) {
    QMessageBox::warning(
        parent, "Invalid USD file",
        "Error: Too few or many variant sets found, expected 1");
    return false;
  }
  if (variantSetNames[0] != "LOD") {
    QMessageBox::warning(parent, "Invalid USD file",
                         "Error: Variant set name is not 'LOD'");
    return false;
  }
  auto lodVariantSet = variantSets.GetVariantSet("LOD");
  auto lodNames = lodVariantSet.GetVariantNames();
  if (lodNames.size() != 3 || lodNames[0] != "LOD0" || lodNames[1] != "LOD1" ||
      lodNames[2] != "LOD2") {
    QMessageBox::warning(
        parent, "Invalid USD file",
        "Error: LOD variant set does not have LOD0, LOD1, LOD2");
    return false;
  }

  // TODO: check for transforms on variants/root prim

  QMessageBox::information(parent, "Success", "USD file verified successfully");
  return true;
}
