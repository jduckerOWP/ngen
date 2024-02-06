#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <stdio.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>

#ifdef NGEN_WITH_SQLITE3
#include <geopackage.hpp>
#endif

//This way we can test the partition, since this doesn't have an explicit MPI dependency
#define NGEN_MPI_ACTIVE
#include "core/Partition_One.hpp"
#include "FileChecker.h"


class PartitionOneTest: public ::testing::Test {

    protected:

    std::vector<std::string> hydro_fabric_paths;

    std::string catchmentDataFile;
    geojson::GeoJSON catchment_collection;

    std::vector<std::string> catchment_subset_ids;
    std::vector<std::string> nexus_subset_ids;

    std::unordered_set<std::string> catchment_ids;
    std::unordered_set<std::string> nexus_ids;

    PartitionOneTest() {} 

    ~PartitionOneTest() override {}

    std::string file_search(const std::vector<std::string> &parent_dir_options, const std::string& file_basename)
    {
        // Build vector of names by building combinations of the path and basename options
        std::vector<std::string> name_combinations;

        // Build so that all path names are tried for given basename before trying a different basename option
        for (auto & path_option : parent_dir_options)
            name_combinations.push_back(path_option + file_basename);

        return utils::FileChecker::find_first_readable(name_combinations);
    }

    void read_file_generate_partition_data()
    {
        const std::string file_path = file_search(hydro_fabric_paths, "catchment_data.geojson");
        if (boost::algorithm::ends_with(catchmentDataFile, "gpkg")) {
          #ifdef NGEN_WITH_SQLITE3
          catchment_collection = ngen::geopackage::read(catchmentDataFile, "divides", catchment_subset_ids);
          #else
          throw std::runtime_error("SQLite3 support required to read GeoPackage files.");
          #endif
        } else {
          catchment_collection = geojson::read(file_path, catchment_subset_ids);
        }

        for(auto& feature: *catchment_collection)
        {
            std::string cat_id = feature->get_id();
            catchment_ids.emplace(cat_id);
            std::string nex_id = feature->get_property("toid").as_string();
            nexus_ids.emplace(nex_id);
        }
    }

    void SetUp() override;

    void TearDown() override;

    void setupArbitraryExampleCase();

};

void PartitionOneTest::SetUp() {
    setupArbitraryExampleCase();
}

void PartitionOneTest::TearDown() {

}

void PartitionOneTest::setupArbitraryExampleCase() {
    hydro_fabric_paths = {
        "data/",
        "./data/",
        "../data/",
        "../../data/",

    };
}

TEST_F(PartitionOneTest, TestPartitionOne) 
{
    read_file_generate_partition_data();

    // Using the PartitionData struct for validating the Partition_One class result
    Partition_One partition_one;
    partition_one.generate_partition(catchment_collection);
    PartitionData data_struct = partition_one.partition_data;

    ASSERT_TRUE(catchment_ids.size() == data_struct.catchment_ids.size());
    ASSERT_TRUE(nexus_ids.size() == data_struct.nexus_ids.size());
}

TEST_F(PartitionOneTest, TestPartitionData_1a)
{
    read_file_generate_partition_data();

    Partition_One partition_one;
    partition_one.generate_partition(catchment_collection);
    PartitionData data_struct = partition_one.partition_data;
    catchment_ids = data_struct.catchment_ids;

    //check catchment partition
    std::vector<std::string> cat_id_vec;
    // convert unordered_set to vector
    for (const auto& id: catchment_ids) {
        cat_id_vec.push_back(id);
    }

    //sort ids
    std::sort(cat_id_vec.begin(), cat_id_vec.end());
    //create set of unique ids
    std::set<std::string> unique(cat_id_vec.begin(), cat_id_vec.end());
    std::set<std::string> duplicates;
    //use set difference to identify all duplicates
    std::set_difference(cat_id_vec.begin(), cat_id_vec.end(), unique.begin(), unique.end(), std::inserter(duplicates, duplicates.end()));
    if( duplicates.size() > 0 ){
        for( auto& id: duplicates){
            std::cout << "catchment "<<id<<" is duplicated!"<<std::endl;
        }
        exit(1);
    }

    ASSERT_TRUE(true);
}

TEST_F(PartitionOneTest, TestPartitionData_1b)
{
    read_file_generate_partition_data();

    Partition_One partition_one;
    partition_one.generate_partition(catchment_collection);
    PartitionData data_struct = partition_one.partition_data;
    nexus_ids = data_struct.nexus_ids;

    //check nexus partition
    std::vector<std::string> nex_id_vec;
    //convert unordered_set to vector
    for (const auto& id: nexus_ids) {
        nex_id_vec.push_back(id);
    }

    //sort ids
    std::sort(nex_id_vec.begin(), nex_id_vec.end());
    //create set of unique ids
    std::set<std::string> unique(nex_id_vec.begin(), nex_id_vec.end());
    std::set<std::string> duplicates;
    //use set difference to identify all duplicates
    std::set_difference(nex_id_vec.begin(), nex_id_vec.end(), unique.begin(), unique.end(), std::inserter(duplicates, duplicates.end()));
    if( duplicates.size() > 0 ){
        for( auto& id: duplicates){
            std::cout << "nexus "<<id<<" is duplicated!"<<std::endl;
        }
        exit(1);
    }
    ASSERT_TRUE(true);
}

#undef NGEN_MPI_ACTIVE
