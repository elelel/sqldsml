#pragma once

#include <sqlite>

#include <tuple>

#include "logging.hpp"
#include "feature_cache.hpp"

// TODO: make parameters copy-based, not ptr?x

namespace sqldsml {
  class feature_common;
  
  class feature_common {
  protected:
    feature_common() :
      id_(0) {
      SQLDSML_HPP_LOG("feature_common(const sqlite::database::type_ptr& db)");
    }

    feature_common(const feature_common& other) :
      id_(other.id_) {
      SQLDSML_HPP_LOG("feature_common(const feature_common& other)");
    }

    feature_common(feature_common&& other) :
      id_(std::move(other.id_)) {
      SQLDSML_HPP_LOG("feature_common(feature_common&& other)");
    }

    ~feature_common() {
      SQLDSML_HPP_LOG("feature_common::~feature_common");
    }

    void swap(feature_common& other) {
      SQLDSML_HPP_LOG("feature_common::swap(feature_common& other)");
      std::swap(id_, other.id_);
    }

    feature_common& operator=(const feature_common& other) {
      SQLDSML_HPP_LOG("feature_common::operator=(const feature_common& other)");
      feature_common tmp(other);
      swap(tmp);
      return *this;
    }

    int64_t& id() {
      return id_;
    }
    
  protected:
    int64_t id_;
  };

  template <typename derived_t,
            typename parameters_t>
  class feature : public feature_common {
  public:
    typedef feature<derived_t, parameters_t> type;
    typedef std::shared_ptr<type> type_ptr;
    typedef derived_t derived_type;
    typedef std::shared_ptr<derived_type> derived_type_ptr;
    typedef parameters_t parameters_type;
    typedef std::shared_ptr<parameters_type> parameters_type_ptr;

    feature(const parameters_type_ptr& parameters) :
      feature_common(),
      parameters_id_(0),
      parameters_(parameters) {
      SQLDSML_HPP_LOG("feature(const sqlite::database::type_ptr& db, const parameters_type_ptr& parameters)");
    }

    feature(const feature& other) :
      feature_common(other),
      parameters_id_(other.parameters_id_),
      parameters_(other.parameters_) {
      SQLDSML_HPP_LOG("feature(const feature& other)");
    }

    feature(feature&& other) :
      feature_common(std::move(other)),
      parameters_id_(other.parameters_id_),
      parameters_(std::move(other.parameters_)) {
      SQLDSML_HPP_LOG("feature(feature&& other)");
    }

    ~feature() {
      SQLDSML_HPP_LOG("feature::~feature");
    }

    void swap(feature& other) {
      SQLDSML_HPP_LOG("feature::swap(feature& other)");
      if (this != &other) {
        feature_common::swap(other);
        std::swap(parameters_id_, other.parameters_id_);
        std::swap(parameters_, other.parameters_);
      }
    }

    feature& operator=(const feature& other) {
      SQLDSML_HPP_LOG("feature::operator=(const feature& other)");
      feature tmp(other);
      swap(tmp);
      return *this;
    }

    const parameters_type_ptr& parameters() const {
      return parameters_;
    }

    int64_t& parameters_id() {
      return parameters_id_;
    }

  protected:
    int64_t parameters_id_;
    parameters_type_ptr parameters_;
  };

}
