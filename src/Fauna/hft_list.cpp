#include "hft_list.h"

using namespace Fauna;

int HftList::find(const std::string& name) const {
  for (int i = 0; i < size(); i++)
    if (vec.at(i).name == name) return i;
  return -1;
}

void HftList::insert(const Hft hft) {
  if (hft.name == "")
    throw std::invalid_argument(
        "HftList::add(): "
        "Hft.name is empty");

  const int pos = find(hft.name);
  if (pos >= 0)
    vec[pos] = hft;  // replace
  else
    vec.push_back(hft);  // append new
}

bool HftList::is_valid(const Parameters& params, std::string& msg) const {
  if (this->size() == 0) {
    msg = "HFT list is empty.";
    return false;
  }

  bool all_valid = true;

  HftList::const_iterator itr = begin();
  while (itr != end()) {
    std::string tmp_msg;
    all_valid &= itr->is_valid(params, tmp_msg);
    msg += tmp_msg;
    itr++;
  }

  return all_valid;
}

void HftList::remove_excluded() {
  std::vector<Hft>::iterator iter = vec.begin();
  while (iter != vec.end())
    if (!iter->is_included)
      iter = vec.erase(iter);
    else
      iter++;
}

