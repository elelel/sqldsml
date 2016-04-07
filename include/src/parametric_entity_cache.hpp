#pragma once

#include <algorithm>
#include <cassert>
#include <sqlite_buffered>

namespace sqldsml {
  template <typename parametric_entity_t>
  class parametric_entity_cache;

  template <typename parametric_entity_t>
  class parametric_entity_cache {
  public:
    typedef parametric_entity_cache<parametric_entity_t> type;
    typedef parametric_entity_t parametric_entity_type;
    typedef typename parametric_entity_type::type_ptr parametric_entity_type_ptr;
    typedef typename parametric_entity_type::parameters_type parameters_type;
    typedef std::set<parametric_entity_type_ptr> parametric_entity_container_type;
    typedef typename parametric_entity_type::id_type id_type;

    template <typename id_fields_container_t,
              typename parameter_fields_container_t>
    parametric_entity_cache(sqlite::database::type_ptr db,
                            const std::string& table_name,
                            const id_fields_container_t& id_fields,
                            const parameter_fields_container_t& parameter_fields) :
      db_(db),
      table_name_(table_name),
      id_fields_(id_fields.begin(), id_fields.end()),
      parameter_fields_(parameter_fields.begin(), parameter_fields.end()) {
    }

    parametric_entity_cache(const type& other) :
      all_entities_(other.all_entities_),
      db_(other.db_),
      table_name_(other.table_name_),
      id_fields_(other.id_fields_),
      parameter_fields_(other.parameter_fields_) {
    }

    parametric_entity_cache(type&& other) :
      all_entities_(std::move(other.all_entities_)),
      db_(std::move(other.db_)),
      table_name_(std::move(other.table_name_)),
      id_fields_(std::move(other.id_fidelds_)),
      parameter_fields_(std::move(other.parameter_fields_)) {
    }

    void swap(type& other) {
      std::swap(all_entities_, other.all_entities_);
      std::swap(db_, other.db_);
      std::swap(table_name_, other.table_name_);
      std::swap(id_fields_, other.id_fields_);
      std::swap(parameter_fields_, other.parameter_fields_);
    }

    type& operator=(const type& other) {
      type tmp(other);
      swap(tmp);
      return *this;
    }

    ~parametric_entity_cache() {
      SQLDSML_HPP_LOG("parametric_entity_cache::~parametric_entity_cache");
    }

    parametric_entity_type_ptr find_by_parameters(const parameters_type& parameters) const {
      auto found = std::find_if(all_entities_.begin(),
				all_entities_.end(), [&parameters](const parametric_entity_type_ptr& f) {
                                  return parameters == f->parameters();
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
        SQLDSML_HPP_LOG("add not found, cache size " + std::to_string(all_entities_.size()));
        parametric_entity_type_ptr f(new parametric_entity_type(parametric_entity));
        all_entities_.insert(f);
        return f;
      } else {
        SQLDSML_HPP_LOG("add found, cache size " + std::to_string(all_entities_.size()));
        return found;
      }
    }

    typename parametric_entity_container_type::iterator begin() {
      return all_entities_.begin();
    }

    typename parametric_entity_container_type::iterator end() {
      return all_entities_.end();
    }

    size_t size() {
      return all_entities_.size();
    }

    void clear() {
      all_entities_.clear();
    }

    parametric_entity_container_type& all_entities() {
      return all_entities_;
    }

    size_t load_ids() {
      typedef decltype(std::tuple_cat(id_type(), parameters_type())) select_record_type;
      typedef sqlite::buffered::input_query_by_keys_base<
        select_record_type,
        parameters_type,
        sqlite::default_value_access_policy> select_query_type;


      assert(id_fields_.size() == 1);
      std::string query_prefix_str;
      for (auto &f : id_fields_) {
        if (query_prefix_str.size() != 0) query_prefix_str += ", ";
        query_prefix_str += "`" + f + "`";
      }

      for (auto &f : parameter_fields_) {
        query_prefix_str += ", `" + f + "`";
      }
      query_prefix_str = "SELECT " + query_prefix_str + " FROM `" + table_name_ + "` WHERE ";

      size_t n_requested = 0;
      select_query_type select(db_, query_prefix_str, parameter_fields_);
      for (auto &f : all_entities_) {
        if (f->id() == id_type()) {
          select.add_key(f->parameters());
          ++n_requested;
        }
      }

      size_t n_selected = 0;
      for (auto r : select) {
        auto found = find_by_parameters(sqlite::tuple_tail(r));
        if (found != nullptr) {
          found->id() = id_type(std::get<0>(r));
          SQLDSML_HPP_LOG(std::string("found for ") + std::to_string(std::get<1>(r)) + ", id = " +
                          std::to_string(std::get<0>(r)));
        }
        ++n_selected;
      }
      assert(n_selected <= n_requested);
      SQLDSML_HPP_LOG(std::string("load_ids() loaded ") + std::to_string(n_selected) + " out of requested " + std::to_string(n_requested));
      return n_selected;
    }

    void create_ids() {
      typedef ::sqlite::buffered::insert_query_base<parameters_type,
                                                    ::sqlite::default_value_access_policy> insert_type;
      insert_type insert(db_, table_name_, parameter_fields_);
      for (auto &f : all_entities_) {
        if (f->id() == id_type()) {
          insert.push_back(f->parameters());
        }
      }
      insert.flush();
    }

    
  private:
    parametric_entity_container_type all_entities_;
    sqlite::database::type_ptr db_;
    std::string table_name_;
    std::vector<std::string> id_fields_;
    std::vector<std::string> parameter_fields_;
  };
}
