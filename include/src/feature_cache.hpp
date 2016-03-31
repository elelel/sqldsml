#pragma once

#include <algorithm>
#include <memory>

#include <sqlite_buffered>

#include "logging.hpp"

namespace sqldsml{
  template <typename feature_t>
  class feature_cache;
  
  template <typename feature_t>
  class feature_cache {
  public:
    typedef feature_t feature_type;
    typedef feature_cache<feature_type> type;
    typedef typename feature_type::type_ptr feature_type_ptr;
    typedef typename feature_type::parameters_type parameters_type;
    typedef typename feature_type::parameters_type_ptr parameters_type_ptr;
    typedef std::set<feature_type_ptr> feature_container_type;

    template <typename parameter_key_fields_container_t>
    feature_cache(sqlite::database::type_ptr db,
                  const std::string& table_name,
                  const std::string& parameters_table_name,
                  const parameter_key_fields_container_t& parameter_key_fields) :
      db_(db),
      table_name_(table_name),
      parameters_table_name_(parameters_table_name),
      parameter_key_fields_(parameter_key_fields.begin(), parameter_key_fields.end()) {
    }

    feature_cache(const type& other) :
      all_features_(other.all_features_),
      db_(other.db_),
      table_name_(other.table_name_),
      parameters_table_name_(other.parameters_table_name_),
      parameter_key_fields_(other.parameter_key_fields_) {
    }

    feature_cache(type&& other) :
      all_features_(std::move(other.all_features_)),
      db_(std::move(other.db_)),
      table_name_(std::move(other.table_name_)),
      parameters_table_name_(std::move(other.parameters_table_name_)),
      parameter_key_fields_(std::move(other.parameter_key_fields_)) {
    }

    void swap(type& other) {
      std::swap(all_features_, other.all_features_);
      std::swap(db_, other.db_);
      std::swap(table_name_, other.table_name_);
      std::swap(parameters_table_name_, other.parameters_table_name_);
      std::swap(parameter_key_fields_, other.parameter_key_fields_);
    }

    type& operator=(const type& other) {
      type tmp(other);
      swap(tmp);
      return *this;
    }

    ~feature_cache() {
      SQLDSML_HPP_LOG("feature_cache::~feature_cache");
    }

    feature_type_ptr find_by_parameters(const parameters_type_ptr parameters_ptr) const {
      auto found = std::find_if(all_features_.begin(),
				all_features_.end(), [&parameters_ptr](const feature_type_ptr& f) {
                                  return parameters_ptr == f->parameters();
                                });
      if (found != all_features_.end()) {
        return *found;
      } else {
        return nullptr;
      }
    }

    feature_type_ptr find_by_parameters(const parameters_type& parameters) const {
      auto found = std::find_if(all_features_.begin(),
				all_features_.end(), [&parameters](const feature_type_ptr& f) {
                                  return parameters == *(f->parameters());
                                });
      if (found != all_features_.end()) {
        return *found;
      } else {
        return nullptr;
      }
    }

    feature_type_ptr find_by_parameters_id(const int64_t parameters_id) const {
      auto found = std::find_if(all_features_.begin(),
				all_features_.end(), [parameters_id](const feature_type_ptr& f) {
                                  return parameters_id == f->parameters_id();
                                });
      if (found != all_features_.end()) {
        return *found;
      } else {
        return nullptr;
      }
    }

    feature_type_ptr add(const feature_type& feature) {
      auto found = find_by_parameters(feature.parameters());
      if (found == nullptr) {
        SQLDSML_HPP_LOG("add not found");
        feature_type_ptr f(new feature_type(feature));
        all_features_.insert(f);
        return f;
      } else {
        SQLDSML_HPP_LOG("add found");
        return found;
      }
    }

    typename feature_container_type::iterator begin() {
      return all_features().begin();
    }

    typename feature_container_type::iterator end() {
      return all_features().end();
    }

    feature_container_type& all_features() {
      return all_features_;
    }


    void load_parameter_ids() {
      const parameters_type dummy;
      typedef decltype(std::tuple_cat(std::tuple<int64_t>(0), dummy)) select_record_type;
      typedef sqlite::buffered::input_query_by_keys_base<
        select_record_type,
        parameters_type,
        sqlite::default_value_access_policy> select_query_type;

      std::string query_prefix_str = "SELECT `id`";
      for (auto &f : parameter_key_fields_) {
        query_prefix_str += ", `" + f + "`";
      }
      query_prefix_str += " FROM `" + parameters_table_name_ + "` WHERE ";

      select_query_type select(db_, query_prefix_str, parameter_key_fields_);
      for (auto &f : all_features_) {
        if (f->parameters_id() == 0) {
          select.add_key(*(f->parameters()));
        }
      }
      for (auto r : select) {
        auto found = find_by_parameters(sqlite::tuple_tail(r));
        if (found != nullptr) {
          found->parameters_id() = std::get<0>(r);
        }
      }
    }

    void create_parameter_ids() {
      typedef ::sqlite::buffered::insert_query_base<parameters_type,
                                                    ::sqlite::default_value_access_policy> insert_type;
      insert_type insert(db_, parameters_table_name_, parameter_key_fields_);
      for (auto &f : all_features_) {
        if (f->parameters_id() == 0) {
          insert.push_back(*(f->parameters()));
        }
      }
      insert.flush();
    }

    void load_ids() {
      const parameters_type dummy;
      typedef std::tuple<int64_t, int64_t> select_record_type;
      typedef sqlite::buffered::input_query_by_keys_base<
        select_record_type,
        parameters_type,
        sqlite::default_value_access_policy> select_query_type;

      std::string query_prefix_str = "SELECT `id`, `parameters_id` ";
      query_prefix_str += " FROM `" + table_name_ + "` WHERE ";

      const std::vector<std::string> feature_search_fields{"parameters_id"};
      select_query_type select(db_, query_prefix_str, feature_search_fields);
      for (auto &f : all_features_) {
        if ((f->id() == 0) && (f->parameters_id() != 0)) {
          select.add_key(std::tuple<int64_t>(f->parameters_id()));
          SQLDSML_HPP_LOG(std::string("feature_cache::load_ids add key ") + std::to_string(f->parameters_id()));
        }
      }
      for (auto r : select) {
        auto found = find_by_parameters_id(std::get<1>(r));
        if (found != nullptr) {
          SQLDSML_HPP_LOG(std::string("feature_cache::load_ids got requested record"));
          found->id() = std::get<0>(r);
        } else {
          SQLDSML_HPP_LOG(std::string("feature_cache::load_ids error - got not requested record"));
        }
      }
    }

    void create_ids() {
      typedef ::sqlite::buffered::insert_query_base<std::tuple<int64_t>,
                                                    ::sqlite::default_value_access_policy> insert_type;
      insert_type insert(db_, table_name_, std::vector<std::string>{"parameters_id"});
      for (auto &f : all_features_) {
        if ((f->id() == 0) && (f->parameters_id() != 0)) {
          insert.push_back(std::tuple<int64_t>(f->parameters_id()));
        }
      }
      insert.flush();
    }

  private:
    std::set<feature_type_ptr> all_features_;
    sqlite::database::type_ptr db_;
    std::string table_name_;
    std::string parameters_table_name_;
    std::vector<std::string> parameter_key_fields_;
  };


}

