#include <gtest/gtest.h>

#define SQLITE_HPP_LOG_FILENAME "sqlite_debug.log"
#define SQLDSML_HPP_LOG_FILENAME "sqldsml_debug.log"

#include <iostream>

#include <sqldsml>

#include <algorithm>
#include <cmath>

#include <sstream>
#include <random>
#include <limits>

class SqldsmlTest : public ::testing::Test {
protected:

  class my_int_feature : public
  ::sqldsml::feature<my_int_feature, std::tuple<int64_t>> {
  public:
    using ::sqldsml::feature<my_int_feature, std::tuple<int64_t>>::feature;
    typedef my_int_feature type;
    typedef std::shared_ptr<type> type_ptr;

    ~my_int_feature() {
      SQLDSML_HPP_LOG("my_int_feature::~my_int_feature");
    }

    static std::string& table_name() {
      static std::string table_name("my_int64_table");
      return table_name;
    }
  };
  
  virtual void SetUp() {
    db = ::sqlite::database::type_ptr(new sqlite::database("test.db"));
  }
  
  virtual void TearDown() {
  }

  typename ::sqlite::database::type_ptr db;

};

TEST_F(SqldsmlTest, ConstructAndCache) {
  ::sqldsml::feature_cache<my_int_feature> my_int_feature_cache(nullptr, "", std::vector<std::string>{""});
  typename my_int_feature::parameters_type_ptr param(new my_int_feature::parameters_type(123));
  typename my_int_feature::parameters_type_ptr param_copy(new my_int_feature::parameters_type(123));
  auto f = my_int_feature_cache.add(my_int_feature(param));
  ASSERT_EQ(my_int_feature_cache.find_by_parameters(f->parameters()), f);
  auto f_param_copy = my_int_feature_cache.add(my_int_feature(param));
  ASSERT_EQ(f_param_copy, f);

  ASSERT_EQ(my_int_feature_cache.all_features().size(), 1);
  for (auto &f : my_int_feature_cache) {
    ASSERT_NE(f.get(), nullptr);
  }
}

TEST_F(SqldsmlTest, SaveLoadParameterIds) {
  // Instances of parameters to be converted to distinct features
  const size_t max_distinct_params = 10000;
  std::vector<typename my_int_feature::parameters_type_ptr> parameter_instances;
  for (int i = 0; i < max_distinct_params; ++i) {
    parameter_instances.push_back(my_int_feature::parameters_type_ptr(new my_int_feature::parameters_type(1000000 + i)));
  }
  // Create sql_database
  std::string parameters_table_name = "test_params";
  sqlite::database::type_ptr db(new sqlite::database::type("test.db"));
  ASSERT_EQ(SQLITE_OK, db->result_code());
  sqlite::query drop_table(db, "DROP TABLE IF EXISTS `" + parameters_table_name + "`");
  drop_table.step();
  ASSERT_EQ(SQLITE_DONE, drop_table.result_code());
  sqlite::query create_table(db, "CREATE TABLE `" + parameters_table_name + "` \
(`id` INTEGER PRIMARY KEY AUTOINCREMENT, `param` INTEGER)");
  create_table.step();
  ASSERT_EQ(SQLITE_DONE, create_table.result_code());
  
  std::vector<std::string> param_fields{"param"};
  sqldsml::feature_cache<my_int_feature> my_int_feature_cache(db, parameters_table_name, param_fields);
  // Add half of the parameters
  for (int i = 0; i < parameter_instances.size() / 2; ++i) {
    my_int_feature f(parameter_instances[i]);
    my_int_feature_cache.add(f);
  }

  auto count_parameter_records = [&db, &parameters_table_name] () {
    sqlite::query count_query(db, "SELECT count(*) FROM `" + parameters_table_name + "`");
    count_query.step();
    int count_half;
    count_query.get(0, count_half);
    return count_half;
  };
  
  my_int_feature_cache.create_parameter_ids();
  auto count_half = count_parameter_records();
  ASSERT_EQ(count_half, parameter_instances.size() / 2);
  my_int_feature_cache.load_parameter_ids();
  for (auto &f : my_int_feature_cache.all_features()) {
    ASSERT_NE(f->parameters_id(), 0);
  }
  
  my_int_feature_cache.create_parameter_ids();
  auto count_half_dups = count_parameter_records();
  ASSERT_EQ(count_half_dups, count_half);

  // Now add all to check how duplicates are handled
  for (int i = 0; i < parameter_instances.size(); ++i) {
    my_int_feature f(parameter_instances[i]);
    my_int_feature_cache.add(f);
  }
  my_int_feature_cache.create_parameter_ids();

  auto count_all = count_parameter_records();
  ASSERT_EQ(count_all, parameter_instances.size()); 
  
}
