#pragma once

#include <sqlite>

#include <tuple>

#include "logging.hpp"
#include "parametric_entity.hpp"

namespace sqldsml {
  template <typename parameters_t>
  class feature;
  
  template <typename parameters_t>
  class feature : public parametric_entity<parameters_t> {
    using parametric_entity<parameters_t>::parametric_entity;
  };
}
