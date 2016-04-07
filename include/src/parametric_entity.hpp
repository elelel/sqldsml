#pragma once

#include <tuple>

#include "logging.hpp"

namespace sqldsml {
  template <typename id_t, typename parameters_t>
  class parametric_entity;
  
  template <typename id_t, typename parameters_t>
  class parametric_entity {
  public:
    typedef parametric_entity<id_t, parameters_t> type;
    typedef parameters_t parameters_type;
    typedef id_t id_type;

    parametric_entity(const parameters_type& parameters) :
      id_(id_type()),
      parameters_(parameters) {
      //      SQLDSML_HPP_LOG("parametric_entity(const parameters_type_ptr& parameters)");
    }

    parametric_entity(const parametric_entity& other) :
      id_(other.id_),
      parameters_(other.parameters_) {
      //      SQLDSML_HPP_LOG("parametric_entity(const parametric_entity& other)");
    }

    parametric_entity(parametric_entity&& other) :
      id_(std::move(other.id_)),
      parameters_(std::move(other.parameters_)) {
      //      SQLDSML_HPP_LOG("parametric_entity(parametric_entity&& other)");
    }

    ~parametric_entity() {
      //      SQLDSML_HPP_LOG("parametric_entity::~parametric_entity");
    }

    void swap(parametric_entity& other) {
      //      SQLDSML_HPP_LOG("parametric_entity::swap(parametric_entity& other)");
      if (this != &other) {
        std::swap(id_, other.id_);
        std::swap(parameters_, other.parameters_);
      }
    }

    parametric_entity& operator=(const parametric_entity& other) {
      //      SQLDSML_HPP_LOG("parametric_entity::operator=(const parametric_entity& other)");
      parametric_entity tmp(other);
      swap(tmp);
      return *this;
    }
    
    id_type& id() {
      return id_;
    }
    
    const parameters_type& parameters() const {
      return parameters_;
    }
    
  protected:
    id_type id_;
    parameters_type parameters_;
  };
}
