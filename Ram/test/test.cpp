#include <gtest/gtest.h>
#include "lexer.hpp"
#include "ram.hpp"
#include "utility.hpp"

namespace fs = std::filesystem;

std::vector<std::string> getFiles(const std::string& directoryPath) 
{
    std::vector<fs::directory_entry> filteredEntries;

    std::copy_if(
        fs::directory_iterator(directoryPath),
        fs::directory_iterator(),
        std::back_inserter(filteredEntries),
        [](const fs::directory_entry& entry) {
            return entry.is_regular_file();
        }
    );

    std::vector<std::string> files;
    std::transform(
        filteredEntries.begin(),
        filteredEntries.end(),
        std::back_inserter(files),
        [](const fs::directory_entry& entry) {
            return entry.path().string();
        }
    );

    return files;
}

std::unordered_map<int, std::string> single_answers;
std::unordered_map<int, std::string> err_file_names;

class MyTestEnvironment final : public ::testing::Environment 
{
public:
    ~MyTestEnvironment() override {}

    void SetUp() override {
        auto single_files = getFiles("../test/data/single/");
        std::sort(std::begin(single_files), std::end(single_files), std::less<std::string>());

        auto error_files = getFiles("../test/data/error/");
        std::sort(std::begin(error_files), std::end(error_files), std::less<std::string>());

        size_t num = 1;
        for(auto&& file: single_files) {
            single_answers.emplace(num++, file);
        }

        num = 1;
        for(auto&& file: error_files) {
            err_file_names.emplace(num++, file);
        }
        // my_impl::utility::print(single_answers);
    }

    void TearDown() override {}
};

::testing::Environment* const my_env = ::testing::AddGlobalTestEnvironment(new MyTestEnvironment);

class Test: public testing::TestWithParam<int> {};
INSTANTIATE_TEST_SUITE_P(test, Test, ::testing::Values(1,2,3,4,5,6,7,8,9,10));


TEST_P(Test, singleTest) 
{
    try {
        auto file = single_answers.at(GetParam());
        auto stream = my_impl::utility::readFile(file);
        auto driver = std::make_unique<my_impl::Driver>(stream);
        driver->parse();

        EXPECT_EQ(std::stoi(driver->output()), GetParam());
        
    }
    catch(const std::exception& excep) {
        FAIL() << excep.what();
    }
}

class ErrorTest: public testing::TestWithParam<int> {};
INSTANTIATE_TEST_SUITE_P(err, ErrorTest, ::testing::Values(1,2,3,4,5,6));


TEST_P(ErrorTest, errTest) 
{
    try {
        auto file = err_file_names.at(GetParam());
        auto stream = my_impl::utility::readFile(file);
        auto driver = std::make_unique<my_impl::Driver>(stream);
        driver->parse();

        auto output = driver->output();

        EXPECT_TRUE(output == "Syntax error\n"             || 
                    output == "Syntax error: expected ;\n" ||
                    output == "Negative index\n" 
                    );
        
    }
    catch(const std::exception& excep) {
        FAIL() << excep.what();
    }
}

int main(int argc, char **argv) 
{

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

