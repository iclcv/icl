// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/DataStore.h>
#include <icl/utils/ProgArg.h>

int main(int n, char **args){
  icl::utils::pa_init(n,args,"-src-type|-s(name) -dst-type|-d(mame)");

  icl::qt::DataStore::list_possible_assignments(icl::utils::pa("-s") ? *icl::utils::pa("-s") : icl::utils::str(""),
                                                icl::utils::pa("-d") ? *icl::utils::pa("-d") : icl::utils::str("") );
}
