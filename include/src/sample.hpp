#pragma once

#include <sqlite>

#include <tuple>

#include "logging.hpp"
#include "parametric_entity.hpp"
#include "parametric_entity_cache.hpp"

namespace sqldsml {
  template <typename parameters_t>
  class sample;
  
  template <typename parameters_t>
  class sample : public parametric_entity< parameters_t> {
    using parametric_entity<parameters_t>::parametric_entity;
  };

  template <typename sample_t>
  class sample_cache : public parametric_entity_cache<sample_t> {
    using parametric_entity_cache<sample_t>::parametric_entity_cache;
  };
  
}
