#pragma once

#include <sqlite>

#include <tuple>

#include "logging.hpp"
#include "parametric_entity.hpp"

namespace sqldsml {
  template <typename derived_t, typename parameters_t>
  class feature;
  
  template <typename derived_t, typename parameters_t>
  class feature : public parametric_entity<feature<derived_t, parameters_t>, parameters_t> {
    using parametric_entity<feature<derived_t, parameters_t>, parameters_t>::parametric_entity;
  };
}
