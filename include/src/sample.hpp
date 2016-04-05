#pragma once

#include <sqlite>

#include <tuple>

#include "logging.hpp"
#include "parametric_entity.hpp"
#include "parametric_entity_cache.hpp"
#include "relational_parametric_entity.hpp"
#include "relational_parametric_entity_cache.hpp"


namespace sqldsml {
  template <typename parameters_t>
  class sample : public parametric_entity< parameters_t> {
    using parametric_entity<parameters_t>::parametric_entity;
  };

  template <typename sample_t>
  class sample_cache : public parametric_entity_cache<sample_t> {
    using parametric_entity_cache<sample_t>::parametric_entity_cache;
  };

  template <typename parameters_t>
  class relational_sample : public relational_parametric_entity< parameters_t> {
    using relational_parametric_entity<parameters_t>::relational_parametric_entity;
  };

  template <typename sample_t>
  class relational_sample_cache : public relational_parametric_entity_cache<sample_t> {
    using relational_parametric_entity_cache<sample_t>::relational_parametric_entity_cache;
  };

}
