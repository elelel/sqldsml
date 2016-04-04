#pragma once

#include <sqlite>

#include <tuple>

#include "logging.hpp"
#include "parametric_entity.hpp"
#include "parametric_entity_cache.hpp"

namespace sqldsml {
  template <typename parameters_t>
  class feature;
  
  template <typename parameters_t>
  class feature : public parametric_entity<parameters_t> {
    using parametric_entity<parameters_t>::parametric_entity;
  };

  template <typename feature_t>
  class feature_cache : public parametric_entity_cache<feature_t> {
    using parametric_entity_cache<feature_t>::parametric_entity_cache;
  };

}
