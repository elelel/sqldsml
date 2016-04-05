#pragma once

#include <tuple>

#include <sqlite>

#include "logging.hpp"

namespace sqldsml {
  template <typename parameters_t>
  class relational_parametric_entity {
  public:
    typedef relational_parametric_entity<parameters_t> type;
    typedef std::shared_ptr<type> type_ptr;
    typedef parameters_t parameters_type;
    typedef std::shared_ptr<parameters_type> parameters_type_ptr;
    typedef std::tuple<int64_t> id_type;
    typedef std::tuple<int64_t> parameters_id_type;

    relational_parametric_entity(const parameters_type_ptr& parameters) :
      id_(id_type()),
      parameters_id_(parameters_id_type()),
      parameters_(parameters) {
      SQLDSML_HPP_LOG("relational_parametric_entity(const sqlite::database::type_ptr& db, const parameters_type_ptr& parameters)");
    }

    relational_parametric_entity(const relational_parametric_entity& other) :
      id_(other.id_),
      parameters_id_(other.parameters_id_),
      parameters_(other.parameters_) {
      SQLDSML_HPP_LOG("relational_parametric_entity(const relational_parametric_entity& other)");
    }

    relational_parametric_entity(relational_parametric_entity&& other) :
      id_(other.id_),
      parameters_id_(other.parameters_id_),
      parameters_(std::move(other.parameters_)) {
      SQLDSML_HPP_LOG("relational_parametric_entity(relational_parametric_entity&& other)");
    }

    ~relational_parametric_entity() {
      SQLDSML_HPP_LOG("relational_parametric_entity::~relational_parametric_entity");
    }

    void swap(relational_parametric_entity& other) {
      SQLDSML_HPP_LOG("relational_parametric_entity::swap(relational_parametric_entity& other)");
      if (this != &other) {
        std::swap(id_, other.id_);
        std::swap(parameters_id_, other.parameters_id_);
        std::swap(parameters_, other.parameters_);
      }
    }

    relational_parametric_entity& operator=(const relational_parametric_entity& other) {
      SQLDSML_HPP_LOG("relational_parametric_entity::operator=(const relational_parametric_entity& other)");
      relational_parametric_entity tmp(other);
      swap(tmp);
      return *this;
    }
    
    id_type& id() {
      return id_;
    }
    
    parameters_id_type& parameters_id() {
      return parameters_id_;
    }
    
    const parameters_type_ptr& parameters() const {
      return parameters_;
    }

  protected:
    id_type id_;
    parameters_id_type parameters_id_;
    parameters_type_ptr parameters_;
  };
}
