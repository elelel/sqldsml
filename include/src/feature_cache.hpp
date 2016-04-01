#pragma once

#include "parametric_entity_cache.hpp"

namespace sqldsml {
  template <typename feature_t>
  class feature_cache : public parametric_entity_cache<feature_t> {
    using parametric_entity_cache<feature_t>::parametric_entity_cache;
  };
}
