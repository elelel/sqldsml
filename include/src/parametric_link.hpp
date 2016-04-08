#pragma once

#include <tuple>

#include <sqlite>

#include "logging.hpp"

namespace sqldsml {
  template <typename entity1_t, typename entity2_t, typename parameters_t>
  class parametric_link {
  public:
    typedef parametric_link<entity1_t, entity2_t, parameters_t> type;
    typedef std::shared_ptr<type> type_ptr;
    typedef entity1_t entity1_type;
    typedef std::shared_ptr<entity1_type> entity1_type_ptr;
    typedef entity2_t entity2_type;
    typedef std::shared_ptr<entity2_type> entity2_type_ptr;
    typedef parameters_t parameters_type;
    typedef std::shared_ptr<parameters_type> parameters_type_ptr;
    typedef decltype(std::tuple_cat(typename entity1_type::id_type(),
                                    typename entity2_type::id_type())) id_type;

    parametric_link(const entity1_type_ptr& entity1,
                    const entity2_type_ptr& entity2,
                    const parameters_type& parameters) :
      entity1_(entity1),
      entity2_(entity2),
      parameters_(parameters) {
      SQLDSML_HPP_LOG("parametric_link(const sqlite::database::type_ptr& db, const parameters_type_ptr& parameters)");
    }

    parametric_link(const parametric_link& other) :
      entity1_(other.entity1_),
      entity2_(other.entity2_),
      parameters_(other.parameters_) {
      SQLDSML_HPP_LOG("parametric_link(const parametric_link& other)");
    }

    parametric_link(parametric_link&& other) :
      entity1_(other.entity1_),
      entity2_(other.entity2_),
      parameters_(std::move(other.parameters_)) {
      SQLDSML_HPP_LOG("parametric_link(parametric_link&& other)");
    }

    ~parametric_link() {
      SQLDSML_HPP_LOG("parametric_link::~parametric_link");
    }

    void swap(parametric_link& other) {
      SQLDSML_HPP_LOG("parametric_link::swap(parametric_link& other)");
      if (this != &other) {
        std::swap(entity1_, other.entity1_);
        std::swap(entity2_, other.entity2_);
        std::swap(parameters_, other.parameters_);
      }
    }

    parametric_link& operator=(const parametric_link& other) {
      SQLDSML_HPP_LOG("parametric_link::operator=(const parametric_link& other)");
      parametric_link tmp(other);
      swap(tmp);
      return *this;
    }
    
    const id_type id() {
      if ((entity1_->id() != typename entity1_type::id_type()) &&
          (entity2_->id() != typename entity2_type::id_type())) {
        return id_type(std::tuple_cat(entity1_->id(), entity2_->id()));
      } else {
        return id_type(std::tuple_cat(typename entity1_type::id_type(), typename entity2_type::id_type()));
      }
    }
    
    const parameters_type& parameters() const {
      return parameters_;
    }

  protected:
    entity1_type_ptr entity1_;
    entity2_type_ptr entity2_;
    parameters_type parameters_;
  };
}
