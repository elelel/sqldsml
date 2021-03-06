#pragma once

#include <algorithm>
#include <memory>

#include <sqlite_buffered>

#include "logging.hpp"

namespace sqldsml{
  template <typename relational_parametric_entity_t>
  class relational_parametric_entity_cache;
  
  template <typename relational_parametric_entity_t>
  class relational_parametric_entity_cache {
  public:
    typedef relational_parametric_entity_t relational_parametric_entity_type;
    typedef relational_parametric_entity_cache<relational_parametric_entity_type> type;
    typedef typename relational_parametric_entity_type::type_ptr relational_parametric_entity_type_ptr;
    typedef typename relational_parametric_entity_type::parameters_type parameters_type;
    typedef typename relational_parametric_entity_type::parameters_type_ptr parameters_type_ptr;
    typedef std::set<relational_parametric_entity_type_ptr> relational_parametric_entity_container_type;
    typedef typename relational_parametric_entity_type::id_type id_type;
    typedef typename relational_parametric_entity_type::parameters_id_type parameters_id_type;

    template <typename parameter_key_fields_container_t>
    relational_parametric_entity_cache(sqlite::database::type_ptr db,
                  const std::string& table_name,
                  const std::string& parameters_table_name,
                  const parameter_key_fields_container_t& parameter_key_fields) :
      db_(db),
      table_name_(table_name),
      parameters_table_name_(parameters_table_name),
      parameter_key_fields_(parameter_key_fields.begin(), parameter_key_fields.end()) {
    }

    relational_parametric_entity_cache(const type& other) :
      all_entities_(other.all_entities_),
      db_(other.db_),
      table_name_(other.table_name_),
      parameters_table_name_(other.parameters_table_name_),
      parameter_key_fields_(other.parameter_key_fields_) {
    }

    relational_parametric_entity_cache(type&& other) :
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

    ~relational_parametric_entity_cache() {
      SQLDSML_HPP_LOG("relational_parametric_entity_cache::~relational_parametric_entity_cache");
    }

    relational_parametric_entity_type_ptr find_by_parameters(const parameters_type_ptr parameters_ptr) const {
      auto found = std::find_if(all_entities_.begin(),
				all_entities_.end(), [&parameters_ptr](const relational_parametric_entity_type_ptr& f) {
                                  return parameters_ptr == f->parameters();
                                });
      if (found != all_entities_.end()) {
        return *found;
      } else {
        return nullptr;
      }
    }

    relational_parametric_entity_type_ptr find_by_parameters(const parameters_type& parameters) const {
      auto found = std::find_if(all_entities_.begin(),
				all_entities_.end(), [&parameters](const relational_parametric_entity_type_ptr& f) {
                                  return parameters == *(f->parameters());
                                });
      if (found != all_entities_.end()) {
        return *found;
      } else {
        return nullptr;
      }
    }

    relational_parametric_entity_type_ptr find_by_parameters_id(const parameters_id_type& parameters_id) const {
      auto found = std::find_if(all_entities_.begin(),
				all_entities_.end(), [parameters_id](const relational_parametric_entity_type_ptr& f) {
                                  return parameters_id == f->parameters_id();
                                });
      if (found != all_entities_.end()) {
        return *found;
      } else {
        return nullptr;
      }
    }

    relational_parametric_entity_type_ptr add(const relational_parametric_entity_type& relational_parametric_entity) {
      auto found = find_by_parameters(relational_parametric_entity.parameters());
      if (found == nullptr) {
        SQLDSML_HPP_LOG("add not found");
        relational_parametric_entity_type_ptr f(new relational_parametric_entity_type(relational_parametric_entity));
        all_entities_.insert(f);
        return f;
      } else {
        SQLDSML_HPP_LOG("add found");
        return found;
      }
    }

    typename relational_parametric_entity_container_type::iterator begin() {
      return all_entities_.begin();
    }

    typename relational_parametric_entity_container_type::iterator end() {
      return all_entities_.end();
    }

    size_t size() {
      return all_entities_.size();
    }

    void clear() {
      all_entities_.clear();
    }

    relational_parametric_entity_container_type& all_entities() {
      return all_entities_;
    }

    void load_parameter_ids() {
      typedef decltype(std::tuple_cat(parameters_id_type(), parameters_type())) select_record_type;
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
        if (f->parameters_id() == parameters_id_type()) {
          select.add_key(*(f->parameters()));
        }
      }
      for (auto r : select) {
        auto found = find_by_parameters(sqlite::tuple_tail(r));
        if (found != nullptr) {
          found->parameters_id() = parameters_id_type(std::get<0>(r));
        }
      }
    }

    void create_parameter_ids() {
      typedef ::sqlite::buffered::insert_query_base<parameters_type,
                                                    ::sqlite::default_value_access_policy> insert_type;
      insert_type insert(db_, parameters_table_name_, parameter_key_fields_);
      for (auto &f : all_entities_) {
        if (f->parameters_id() == parameters_id_type()) {
          insert.push_back(*(f->parameters()));
        }
      }
      insert.flush();
    }

    void load_ids() {
      typedef decltype(std::tuple_cat(id_type(), parameters_id_type())) select_record_type;
      typedef sqlite::buffered::input_query_by_keys_base<
        select_record_type,
        parameters_type,
        sqlite::default_value_access_policy> select_query_type;

      std::string query_prefix_str = "SELECT `id`, `parameters_id` ";
      query_prefix_str += " FROM `" + table_name_ + "` WHERE ";

      const std::vector<std::string> search_fields{"parameters_id"};
      select_query_type select(db_, query_prefix_str, search_fields);
      for (auto &f : all_entities_) {
        if ((f->id() == id_type()) &&
            (f->parameters_id() != parameters_id_type())) {
          select.add_key(f->parameters_id());
          //  SQLDSML_HPP_LOG(std::string("relational_parametric_entity_cache::load_ids add key ") + std::to_string(f->parameters_id()));
        }
      }
      for (auto r : select) {
        auto found = find_by_parameters_id(parameters_id_type(std::get<1>(r)));
        if (found != nullptr) {
          SQLDSML_HPP_LOG(std::string("relational_parametric_entity_cache::load_ids got requested record"));
          found->id() = id_type(std::get<0>(r));
        } else {
          SQLDSML_HPP_LOG(std::string("relational_parametric_entity_cache::load_ids error - got not requested record"));
        }
      }
    }

    void create_ids() {
      typedef ::sqlite::buffered::insert_query_base<parameters_id_type,
                                                    ::sqlite::default_value_access_policy> insert_type;
      insert_type insert(db_, table_name_, std::vector<std::string>{"parameters_id"});
      for (auto &f : all_entities_) {
        if ((f->id() == id_type()) &&
            (f->parameters_id() != parameters_id_type())) {
          insert.push_back(parameters_id_type(f->parameters_id()));
        }
      }
      insert.flush();
    }

  private:
    relational_parametric_entity_container_type all_entities_;
    sqlite::database::type_ptr db_;
    std::string table_name_;
    std::string parameters_table_name_;
    std::vector<std::string> parameter_key_fields_;
  };

}

