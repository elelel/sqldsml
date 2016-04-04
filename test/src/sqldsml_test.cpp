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
  ::sqldsml::feature<std::tuple<int64_t>> {
  public:
    using ::sqldsml::feature<std::tuple<int64_t>>::feature;
    typedef my_int_feature type;
    typedef std::shared_ptr<type> type_ptr;

    ~my_int_feature() {
      SQLDSML_HPP_LOG("my_int_feature::~my_int_feature");
    }
  };

  class my_int_sample : public
  ::sqldsml::sample<std::tuple<int64_t>> {
  public:
    using ::sqldsml::sample<std::tuple<int64_t>>::sample;
    typedef my_int_sample type;
    typedef std::shared_ptr<type> type_ptr;
  };

  void create_parameters_table() {
    sqlite::query drop_table(db, "DROP TABLE IF EXISTS `" + parameters_table_name + "`");
    drop_table.step();
    ASSERT_EQ(SQLITE_DONE, drop_table.result_code());
    sqlite::query create_table(db, "CREATE TABLE `" + parameters_table_name + "` \
(`id` INTEGER UNIQUE PRIMARY KEY AUTOINCREMENT, `param` INTEGER)");
    create_table.step();
    ASSERT_EQ(SQLITE_DONE, create_table.result_code());
  }

  void create_sample_to_id_table() {
    sqlite::query drop_table(db, "DROP TABLE IF EXISTS `" + sample_int_to_id_table_name + "`");
    drop_table.step();
    ASSERT_EQ(SQLITE_DONE, drop_table.result_code());
    sqlite::query create_table(db, "CREATE TABLE `" + sample_int_to_id_table_name + "` \
(`id` INTEGER UNIQUE PRIMARY KEY AUTOINCREMENT, `param` INTEGER)");
    create_table.step();
    ASSERT_EQ(SQLITE_DONE, create_table.result_code());
  }

  void create_sample_table() {
    sqlite::query drop_table(db, "DROP TABLE IF EXISTS `" + sample_table_name + "`");
    drop_table.step();
    ASSERT_EQ(SQLITE_DONE, drop_table.result_code());
    sqlite::query create_table(db, "CREATE TABLE `" + sample_table_name + "` \
(`id` INTEGER UNIQUE PRIMARY KEY AUTOINCREMENT, `parameters_id` INTEGER NOT NULL)");
    create_table.step();
    ASSERT_EQ(SQLITE_DONE, create_table.result_code());
  }

  int count_parameter_records() {
    sqlite::query count_query(db, "SELECT count(*) FROM `" + parameters_table_name + "`");
    count_query.step();
    int count_half;
    count_query.get(0, count_half);
    return count_half;
  }

  void create_feature_table() {
    sqlite::query drop_table(db, "DROP TABLE IF EXISTS `" + feature_table_name + "`");
    drop_table.step();
    ASSERT_EQ(SQLITE_DONE, drop_table.result_code());
    sqlite::query create_table(db, "CREATE TABLE `" + feature_table_name + "` \
(`id` INTEGER PRIMARY KEY AUTOINCREMENT, `parameters_id` INTEGER NOT NULL)");
    create_table.step();
    ASSERT_EQ(SQLITE_DONE, create_table.result_code());
  }

  virtual void SetUp() {
    db = ::sqlite::database::type_ptr(new sqlite::database("test.db"));
  }
  
  virtual void TearDown() {
  }

  typename ::sqlite::database::type_ptr db;
  std::string feature_table_name = "test_features";
  std::string parameters_table_name = "test_params";
  std::string sample_table_name = "test_samples";
  std::string sample_int_to_id_table_name = "sample_map";
    
};

TEST_F(SqldsmlTest, ConstructAndCache) {
  ::sqldsml::feature_cache<my_int_feature> my_int_feature_cache(nullptr, "", "", std::vector<std::string>{""});
  typename my_int_feature::parameters_type_ptr param(new my_int_feature::parameters_type(123));
  typename my_int_feature::parameters_type_ptr param_copy(new my_int_feature::parameters_type(123));
  auto f = my_int_feature_cache.add(my_int_feature(param));
  ASSERT_EQ(my_int_feature_cache.find_by_parameters(f->parameters()), f);
  auto f_param_copy = my_int_feature_cache.add(my_int_feature(param));
  ASSERT_EQ(f_param_copy, f);

  ASSERT_EQ(my_int_feature_cache.all_entities().size(), 1);
  for (auto &f : my_int_feature_cache) {
    ASSERT_NE(f.get(), nullptr);
  }
}
/*
TEST_F(SqldsmlTest, SaveLoadParameterIds) {
  // Instances of parameters to be converted to distinct features
  const size_t max_distinct_params = 10000;
  std::vector<typename my_int_feature::parameters_type_ptr> parameter_instances;
  for (int i = 0; i < max_distinct_params; ++i) {
    parameter_instances.push_back(my_int_feature::parameters_type_ptr(new my_int_feature::parameters_type(1000000 + i)));
  }

  ASSERT_EQ(SQLITE_OK, db->result_code());
  create_parameters_table();
  
  std::vector<std::string> param_fields{"param"};
  sqldsml::feature_cache<my_int_feature> my_int_feature_cache(db, "", parameters_table_name, param_fields);
  // Add half of the parameters
  for (int i = 0; i < parameter_instances.size() / 2; ++i) {
    my_int_feature f(parameter_instances[i]);
    my_int_feature_cache.add(f);
  }

  my_int_feature_cache.create_parameter_ids();
  auto count_half = count_parameter_records();
  ASSERT_EQ(count_half, parameter_instances.size() / 2);
  my_int_feature_cache.load_parameter_ids();
  my_int_feature::parameters_id_type null;
  for (auto &f : my_int_feature_cache.all_entities()) {
    ASSERT_NE(f->parameters_id(), null);
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

TEST_F(SqldsmlTest, SaveAndLoadFeatureIds) {
  const size_t max_distinct_params = 10000;
  std::vector<typename my_int_feature::parameters_type_ptr> parameter_instances;
  for (int i = 0; i < max_distinct_params; ++i) {
    parameter_instances.push_back(my_int_feature::parameters_type_ptr(new my_int_feature::parameters_type(1000000 + i)));
  }

  ASSERT_EQ(SQLITE_OK, db->result_code());
  create_parameters_table();
  create_feature_table();
  std::vector<std::string> param_fields{"param"};
  sqldsml::feature_cache<my_int_feature> my_int_feature_cache(db, feature_table_name, parameters_table_name, param_fields);
  
  for (int i = 0; i < parameter_instances.size(); ++i) {
    my_int_feature f(parameter_instances[i]);
    my_int_feature_cache.add(f);
  }
  my_int_feature_cache.create_parameter_ids();
  my_int_feature_cache.load_parameter_ids();
  my_int_feature_cache.create_ids();
  my_int_feature_cache.load_ids();

  my_int_feature::id_type null_id;
  my_int_feature::parameters_id_type null_parameters_id;
  for (auto &f : my_int_feature_cache.all_entities()) {
    ASSERT_NE(f->parameters_id(), null_id);
    ASSERT_NE(f->id(), null_parameters_id);
  }
}
*/
TEST_F(SqldsmlTest, CreateSamplesAndLinks) {
  const size_t max_samples = 3000;
  const size_t max_features = 3000;

  typedef std::vector<double> raw_sample_type;

  std::uniform_real_distribution<double> uniform_real(0, 1);
  std::uniform_int_distribution<int> sample_features_on(20, 50);
  std::uniform_int_distribution<int> feature_index(0, max_features-1);
  std::default_random_engine re;
  
  std::vector<raw_sample_type> raw_dataset;
  for (int i = 0; i < max_samples; ++i) {
    // Create sample
    raw_sample_type s;
    s.resize(max_features);
    // Choose how many sample's features will be "on" (emulate sparsity)
    int features_on = sample_features_on(re);
    for (int k = 0; k < features_on; ++k) {
      // Choose which feature to turn "on"
      int idx = feature_index(re);
      // Choose feature's value
      s[idx] = uniform_real(re);
    }
    raw_dataset.push_back(s);
  }
    
  create_parameters_table();
  create_feature_table();
  create_sample_to_id_table();
  create_sample_table();
  
  std::vector<std::string> param_fields{"param"};
  sqldsml::feature_cache<my_int_feature> feature_cache(db, feature_table_name, parameters_table_name, param_fields);
  sqldsml::sample_cache<my_int_sample> sample_cache(db, sample_table_name, sample_int_to_id_table_name, param_fields);

  auto flush = [&feature_cache, &sample_cache] () {
      feature_cache.load_parameter_ids();
      feature_cache.create_parameter_ids();
      feature_cache.load_parameter_ids();
      
      feature_cache.load_ids();
      feature_cache.create_ids();
      feature_cache.load_ids();

      sample_cache.load_parameter_ids();
      sample_cache.create_parameter_ids();
      sample_cache.load_parameter_ids();

      sample_cache.load_ids();
      sample_cache.create_ids();
      sample_cache.load_ids();
  };
  
  // Pretend we are scanning a dataset to generate features
  for (int k = 0; k < max_samples; ++k) {
    std::cout << "Sample " << std::to_string(k) << " \n";
    std::shared_ptr<std::tuple<int64_t>> k_param(new std::tuple<int64_t>(k));
    
    auto s = sample_cache.add(my_int_sample(k_param));
    for (int i = 0; i < max_features; ++i) {
      if (raw_dataset[k][i] != 0) {
        std::shared_ptr<std::tuple<int64_t>> i_param(new std::tuple<int64_t>(i));
        auto f = feature_cache.add(my_int_feature(i_param));
      }
    }

    if (k % 600 == 0) {
      flush();
      sample_cache.clear();
    }
  }
  std::cout << "End, flushing\n";
  flush();
  std::cout << "Flushed\n";
}
