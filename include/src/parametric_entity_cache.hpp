#pragma once

#include <algorithm>
#include <memory>

#include <sqlite_buffered>

#include "logging.hpp"

namespace sqldsml{
  template <typename parametric_entity_t>
  class parametric_entity_cache;
  
  template <typename parametric_entity_t>
  class parametric_entity_cache {
  public:
    typedef parametric_entity_t parametric_entity_type;
    typedef parametric_entity_cache<parametric_entity_type> type;
    typedef typename parametric_entity_type::type_ptr parametric_entity_type_ptr;
    typedef typename parametric_entity_type::parameters_type parameters_type;
    typedef typename parametric_entity_type::parameters_type_ptr parameters_type_ptr;
    typedef std::set<parametric_entity_type_ptr> parametric_entity_container_type;

    template <typename parameter_key_fields_container_t>
    parametric_entity_cache(sqlite::database::type_ptr db,
                  const std::string& table_name,
                  const std::string& parameters_table_name,
                  const parameter_key_fields_container_t& parameter_key_fields) :
      db_(db),
      table_name_(table_name),
      parameters_table_name_(parameters_table_name),
      parameter_key_fields_(parameter_key_fields.begin(), parameter_key_fields.end()) {
    }

    parametric_entity_cache(const type& other) :
      all_entities_(other.all_entities_),
      db_(other.db_),
      table_name_(other.table_name_),
      parameters_table_name_(other.parameters_table_name_),
      parameter_key_fields_(other.parameter_key_fields_) {
    }

    parametric_entity_cache(type&& other) :
      all_entities_(std::move(other.all_entities_)),
      db_(std::move(other.db_)),
      table_name_(std::move(other.table_name_)),
      parameters_table_name_(std::move(other.parameters_table_name_)),
      parameter_key_fields_(std::move(other.parameter_key_fields_)) {
    }

    void swap(type& other) {
      std::swap(all_entities_, other.all_entities_);
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

    ~parametric_entity_cache() {
      SQLDSML_HPP_LOG("parametric_entity_cache::~parametric_entity_cache");
    }

    parametric_entity_type_ptr find_by_parameters(const parameters_type_ptr parameters_ptr) const {
      auto found = std::find_if(all_entities_.begin(),
				all_entities_.end(), [&parameters_ptr](const parametric_entity_type_ptr& f) {
                                  return parameters_ptr == f->parameters();
                                });
      if (found != all_entities_.end()) {
        return *found;
      } else {
        return nullptr;
      }
    }

    parametric_entity_type_ptr find_by_parameters(const parameters_type& parameters) const {
      auto found = std::find_if(all_entities_.begin(),
				all_entities_.end(), [&parameters](const parametric_entity_type_ptr& f) {
                                  return parameters == *(f->parameters());
                                });
      if (found != all_entities_.end()) {
        return *found;
      } else {
        return nullptr;
      }
    }

    parametric_entity_type_ptr find_by_parameters_id(const int64_t parameters_id) const {
      auto found = std::find_if(all_entities_.begin(),
				all_entities_.end(), [parameters_id](const parametric_entity_type_ptr& f) {
                                  return parameters_id == f->parameters_id();
                                });
      if (found != all_entities_.end()) {
        return *found;
      } else {
        return nullptr;
      }
    }

    parametric_entity_type_ptr add(const parametric_entity_type& parametric_entity) {
      auto found = find_by_parameters(parametric_entity.parameters());
      if (found == nullptr) {
        SQLDSML_HPP_LOG("add not found");
        parametric_entity_type_ptr f(new parametric_entity_type(parametric_entity));
        all_entities_.insert(f);
        return f;
      } else {
        SQLDSML_HPP_LOG("add found");
        return found;
      }
    }

    typename parametric_entity_container_type::iterator begin() {
      return all_entities_.begin();
    }

    typename parametric_entity_container_type::iterator end() {
      return all_entities_.end();
    }

    parametric_entity_container_type& all_entities() {
      return all_entities_;
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
      for (auto &f : all_entities_) {
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
      for (auto &f : all_entities_) {
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

      const std::vector<std::string> search_fields{"parameters_id"};
      select_query_type select(db_, query_prefix_str, search_fields);
      for (auto &f : all_entities_) {
        if ((f->id() == 0) && (f->parameters_id() != 0)) {
          select.add_key(std::tuple<int64_t>(f->parameters_id()));
          SQLDSML_HPP_LOG(std::string("parametric_entity_cache::load_ids add key ") + std::to_string(f->parameters_id()));
        }
      }
      for (auto r : select) {
        auto found = find_by_parameters_id(std::get<1>(r));
        if (found != nullptr) {
          SQLDSML_HPP_LOG(std::string("parametric_entity_cache::load_ids got requested record"));
          found->id() = std::get<0>(r);
        } else {
          SQLDSML_HPP_LOG(std::string("parametric_entity_cache::load_ids error - got not requested record"));
        }
      }
    }

    void create_ids() {
      typedef ::sqlite::buffered::insert_query_base<std::tuple<int64_t>,
                                                    ::sqlite::default_value_access_policy> insert_type;
      insert_type insert(db_, table_name_, std::vector<std::string>{"parameters_id"});
      for (auto &f : all_entities_) {
        if ((f->id() == 0) && (f->parameters_id() != 0)) {
          insert.push_back(std::tuple<int64_t>(f->parameters_id()));
        }
      }
      insert.flush();
    }

  private:
    parametric_entity_container_type all_entities_;
    sqlite::database::type_ptr db_;
    std::string table_name_;
    std::string parameters_table_name_;
    std::vector<std::string> parameter_key_fields_;
  };

}

