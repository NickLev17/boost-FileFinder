#include <boost/program_options.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <boost/crc.hpp>
using namespace std;
namespace po = boost::program_options;
namespace fs = filesystem;

vector<string> parsDirect(string &direct)
{
    vector<string> directory;

    stringstream ss(direct);
    string item;

    while (std::getline(ss, item, ','))
    {
        directory.push_back(item);
    }

    return directory;
}

string parsNameFile(string file)
{

    for (auto iter = file.end(); iter != file.begin(); --iter)
    {
        if (*iter == '/')
        {

            string name(iter + 1, file.end());
            return name;
        }
    }
    return string("None");
}

class FileFinder
{
public:
    string maska;
    FileFinder(vector<string> &_path, vector<string> &_exceptPath, const string &maskFile)
    {
        maska = maskFile;
        workPath.clear();
        path_and_file.clear();
        sizeFiles.clear();
        workPath = _path;

        if (!_exceptPath.empty())
        {
            workPath.erase(std::remove_if(workPath.begin(), workPath.end(),
                                          [&](const std::string &w)
                                          {
                                              return std::find(_exceptPath.begin(), _exceptPath.end(), w) != _exceptPath.end();
                                          }),
                           workPath.end());

            for (const auto &w : workPath)
                std::cout << w << std::endl;
        }
    }

    void findFileInPath(size_t _sizeFile)
    {

        auto iter = workPath.begin();
        while (iter != workPath.end())
        {
            for (const auto &entry : fs::directory_iterator(iter.base()->data()))
            {

                uintmax_t temp = getSizeFile(entry.path());

                if (temp >= _sizeFile)
                {
                    if (matchMask(entry.path(), maska))
                    {

                        sizeFiles.push_back(temp);
                        path_and_file.push_back({iter.base()->data(), parsNameFile(entry.path())});
                    }
                }
            }

            ++iter;
        }
    }

    uintmax_t getSizeFile(const string &file)
    {
        fs::path file_path(file);

        try
        {
            if (fs::exists(file_path) && fs::is_regular_file(file_path))
            {
                uintmax_t size = fs::file_size(file_path);

                return size;
            }
            else
            {
                // std::cerr << "Файл не найден или не является обычным файлом\n";
            }
        }
        catch (const fs::filesystem_error &e)
        {
            std::cerr << "Ошибка при определении размера файла: " << e.what() << std::endl;
        }

        return 0;
    }
    bool matchMask(const std::string &filename, const std::string &pattern)
    {

        std::string regex_str = "^" + boost::regex_replace(pattern, boost::regex(R"([\.\^\$\|\(\)\[\]\+\{\}\\])"), R"(\\$&)") + "$";

        regex_str = boost::regex_replace(regex_str, boost::regex("\\*"), ".*");
        regex_str = boost::regex_replace(regex_str, boost::regex("\\?"), ".");

        boost::regex re(regex_str, boost::regex::icase);
        return boost::regex_match(filename, re);
    }

    vector<string> workPath;

    vector<std::pair<string, string>> path_and_file;
    vector<uintmax_t> sizeFiles;
};

class FileRider
{
public:
    FileRider(size_t block) : blockSize{block}
    {
    }

    void readFile(const std::string &str)
    {
        std::ifstream file(str, std::ios::binary | std::ios::ate);
        if (!file)
        {
            std::cerr << "ERROR open file\n";
            return;
        }

        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::size_t paddedSize = ((fileSize + blockSize - 1) / blockSize) * blockSize;

        std::vector<char> buffer(paddedSize, '\0');

        if (!file.read(buffer.data(), fileSize))
        {
            std::cerr << "ERROR reading file\n";
            return;
        }
        file.close();

        hashData.clear();
        std::string temp;

        for (std::size_t offset = 0; offset < paddedSize; offset += blockSize)
        {
            temp.assign(buffer.data() + offset, blockSize);
            hashData.push_back(setHeshData(temp));
        }
    }

    unsigned int setHeshData(const string &value)
    {
        string temp = value;
        boost::crc_32_type crc32;
        crc32.process_bytes(value.data(), value.size());
        return crc32.checksum();
    }
    vector<unsigned int> hashData;
    size_t sizeFile;
    size_t blockSize;
};

class Comparator
{

public:
    FileFinder findFile;
    size_t blockSize;
    Comparator(const FileFinder &finder, size_t _blockSize) : findFile(finder), blockSize{_blockSize}
    {
    }

    void createCollection()
    {

        FileRider rider(blockSize);
        auto iter = findFile.sizeFiles.begin();

        for (const auto &[k, m] : findFile.path_and_file)
        {
            rider.readFile(k + "/" + m);
            collection col(std::make_pair(k, m), *iter, rider.hashData);
            vec.push_back(col);
            ++iter;
        }
    }

    void showCollection()
    {

        for (const auto &c : vec)
        {
            auto &p = boost::fusion::at_c<0>(c);
            auto &num = boost::fusion::at_c<1>(c);
            auto &vec_uint = boost::fusion::at_c<2>(c);

            cout << "Pair: (" << p.first << ", " << p.second << "), ";
            cout << "size_t: " << std::dec << num << ", ";
            cout << "vector<unsigned int>: [";

            for (size_t i = 0; i < vec_uint.size(); ++i)
            {
                cout << std::hex << vec_uint[i];
                if (i + 1 < vec_uint.size())
                    cout << ", ";
            }
            cout << "]" << endl;
        }
    }

    void compareCollection()
    {
        std::vector<bool> processed(vec.size(), false);

        for (size_t i = 0; i < vec.size(); ++i)
        {
            if (processed[i])
            {
                continue;
            }

            equal.clear();
            processed[i] = true;

            for (size_t j = i + 1; j < vec.size(); ++j)
            {
                if (!processed[j] && compareHesh(boost::fusion::at_c<2>(vec[i]), boost::fusion::at_c<2>(vec[j])))
                {
                    equal.push_back(vec[j]);
                    processed[j] = true;
                }
            }

            if (!equal.empty())
            {
                equal.push_back(vec[i]);
                std::cout << "Group equal elements:\n";
                for (auto &eq : equal)
                {
                    shwowElementCollection(eq);
                }
                std::cout << "\n";
            }
        }
    }

    bool compareHesh(const vector<unsigned int> &left, const vector<unsigned int> &right)
    {
        if (left.size() != right.size())
        {
            return false;
        }

        for (size_t i{}; i < left.size(); i++)
        {

            if (left.at(i) != right.at(i))
            {
                return false;
            }
        }

        return true;
    }

    using collection = boost::fusion::vector<pair<string, string>, uintmax_t, vector<unsigned int>>;
    vector<collection> vec;
    vector<collection> equal;

    void shwowElementCollection(const collection &collect)
    {
        auto &p = boost::fusion::at_c<0>(collect);
        auto &num = boost::fusion::at_c<1>(collect);
        auto &vec_uint = boost::fusion::at_c<2>(collect);

        cout << "Path: " << p.first << " file:  " << p.second << ", ";
        cout << "size: " << std::dec << num << ", ";
        cout << "Hash: [";
        for (size_t i = 0; i < vec_uint.size(); ++i)
        {
            cout << std::hex << vec_uint[i];
            if (i + 1 < vec_uint.size())
                cout << ", ";
        }
        cout << "]" << endl;
    }
};

int main(int argc, char *argv[])
{

    string direct;
    string exceptionDirect;
    size_t sizeFile;
    string maskFile;
    size_t blockSize;
    string hAlgorithm;
    int levelScan;

    po::options_description desc("Options");
    desc.add_options()("help,h", "Show help")(
        "direct,d", po::value<string>(&direct), "work directory")("exceptiondirect,exd", po::value<string>(&exceptionDirect), "exception directory")("levelscan,l", po::value<int>(&levelScan)->default_value(0), "level scaning")("sizefile,sf", po::value<size_t>(&sizeFile)->default_value(1), "size file")("maskfile,m", po::value<string>(&maskFile)->default_value("*"), "mask file")("blocksize,bs", po::value<size_t>(&blockSize)->default_value(1), "size file")("hashalgorithm,ha", po::value<string>(&hAlgorithm)->default_value("crc32"), "hash algorithm");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        cout << desc << endl;
        return 0;
    }

    auto path = parsDirect(direct);

    auto exceptionPath = parsDirect(exceptionDirect);

    FileFinder filefinder(path, exceptionPath, maskFile);
    filefinder.findFileInPath(sizeFile);

    Comparator compar(filefinder, blockSize);
    compar.createCollection();
    compar.compareCollection();

    return 0;
}
