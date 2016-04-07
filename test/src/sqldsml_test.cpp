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
  };

  class my_int_sample : public
  ::sqldsml::sample<std::tuple<int64_t>,
                    std::tuple<int64_t>> {
  public:
    using ::sqldsml::sample<std::tuple<int64_t>,
                            std::tuple<int64_t>>::sample;
    typedef my_int_sample type;
    typedef std::shared_ptr<type> type_ptr;
  };

  class my_real_value : public
  ::sqldsml::value<my_int_sample, my_int_feature, double> {
  public:
    using ::sqldsml::value<my_int_sample, my_int_feature, double>::value;
    typedef my_real_value type;
    typedef std::shared_ptr<type> type_ptr;
  };

  void create_feature_table() {
    sqlite::query drop_table(db, "DROP TABLE IF EXISTS `" + feature_table_name + "`");
    drop_table.step();
    ASSERT_EQ(SQLITE_DONE, drop_table.result_code());
    sqlite::query create_table(db, "CREATE TABLE `" + feature_table_name + "` \
(`id` INTEGER PRIMARY KEY AUTOINCREMENT, `" + feature_parameter_fields[0] +"` INTEGER NOT NULL)");
    create_table.step();
    ASSERT_EQ(SQLITE_DONE, create_table.result_code());
  }

  void create_sample_table() {
    sqlite::query drop_table(db, "DROP TABLE IF EXISTS `" + sample_table_name + "`");
    drop_table.step();
    ASSERT_EQ(SQLITE_DONE, drop_table.result_code());
    sqlite::query create_table(db, "CREATE TABLE `" + sample_table_name + "` \
(`id` INTEGER PRIMARY KEY AUTOINCREMENT, `" + sample_parameter_fields[0] + "` INTEGER NOT NULL)");
    create_table.step();
    ASSERT_EQ(SQLITE_DONE, create_table.result_code());
  }

  void create_value_table() {
    sqlite::query drop_table(db, "DROP TABLE IF EXISTS `" + value_table_name + "`");
    drop_table.step();
    ASSERT_EQ(SQLITE_DONE, drop_table.result_code());
    sqlite::query create_table(db, "CREATE TABLE `" + value_table_name + "` \
(`sample_id` INTEGER, `feature_id` INTEGER, `" + value_parameter_fields[0] + "` INTEGER NOT NULL, \
PRIMARY KEY(sample_id, feature_id) )");
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
  std::vector<std::string> feature_id_fields = {"id"};
  std::vector<std::string> feature_parameter_fields = {"feature_index"};
  std::string sample_table_name = "test_samples";
  std::vector<std::string> sample_id_fields = {"id"};
  std::vector<std::string> sample_parameter_fields = {"sample_natural_id"};
  std::string value_table_name = "test_values";
  std::vector<std::string> value_id_fields = {"sample_id", "feature_id"};
  std::vector<std::string> value_parameter_fields = {"value"};
};

TEST_F(SqldsmlTest, CreateSamplesAndLinks) {
  const size_t max_samples = 200;
  const size_t max_features = 200;
  const size_t flush_every = 53;

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
    
  create_feature_table();
  create_sample_table();
  create_value_table();
  
  sqldsml::feature_cache<my_int_feature> feature_cache(db, feature_table_name,
                                                       feature_id_fields,
                                                       feature_parameter_fields);
  sqldsml::sample_cache<my_int_sample> sample_cache(db, sample_table_name,
                                                    sample_id_fields,
                                                    sample_parameter_fields);
  
  sqldsml::value_cache<my_real_value> value_cache(db, value_table_name,
                                                  value_id_fields,
                                                  value_parameter_fields);
  
  auto flush = [&feature_cache, &sample_cache] () {
    std::cout << "Flushing\n";
    feature_cache.load_ids();
    feature_cache.create_ids();
    feature_cache.load_ids();
    for (auto e : feature_cache.all_entities()) {
      ASSERT_NE(std::get<0>(e->id()), 0);
    }

    SQLDSML_HPP_LOG(std::string("preliminary load"));
    sample_cache.load_ids();
    SQLDSML_HPP_LOG(std::string("create"));
    sample_cache.create_ids();
    SQLDSML_HPP_LOG(std::string("load"));
    sample_cache.load_ids();
  };
  
  // Pretend we are scanning a dataset to generate features
  for (int k = 0; k < max_samples; ++k) {
    SQLDSML_HPP_LOG(std::string("Adding sample ") +  std::to_string(k));
    auto s = sample_cache.add(my_int_sample(std::tuple<int64_t>(k)));
    for (int i = 0; i < max_features; ++i) {
      if (raw_dataset[k][i] != 0) {
        SQLDSML_HPP_LOG(std::string("Adding feature ") +  std::to_string(i));
        auto f = feature_cache.add(my_int_feature(std::tuple<int64_t>(i)));
        
      }
    }

    if ((k + 1) % flush_every == 0) {
      flush();
      sample_cache.clear();
    }
  }
  std::cout << "End, flushing\n";
  flush();
  std::cout << "Flushed\n";
}

