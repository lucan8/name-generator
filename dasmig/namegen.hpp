#include "random.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

// Written by Diego Dasso Migotto - diegomigotto at hotmail dot com
namespace dasmig
{
// The name generator generates names and surnames, allowing requests specifying
// gender and culture. Internally it peaks from a list of popular names and
// surnames in each country.
class ng
{
  public:
    // Copy/move constructors can be deleted since they are not going to be
    // used due to singleton pattern.
    ng(const ng&) = delete;
    ng(ng&&) = delete;
    ng& operator=(const ng&) = delete;
    ng& operator=(ng&&) = delete;

    // Culture representing a country or a broader group (e.g danish, german)
    enum class culture
    {
        american,
        argentinian,
        australian,
        brazilian,
        british,
        bulgarian,
        canadian,
        chinese,
        danish,
        finnish,
        french,
        german,
        kazakh,
        mexican,
        norwegian,
        polish,
        portuguese,
        russian,
        spanish,
        swedish,
        turkish,
        ukrainian,
        any
    };

    // Simple gender enum to distinguish between male and female names.
    enum class gender
    {
        m,
        f,
        any
    };

    // Internal class used by name generator to allow for chained calls when
    // building a full name.
    class name
    {
      public:
        // Method to append a name to the generated name (useful for composed
        // forenames).
        name append_name()
        {
            _internal_container.push_back(
                ng::instance().solver(true, _gender, _culture));
            _internal_string.append(L" ").append(_internal_container.back());

            return *this;
        }

        // Method to append a name to the generated name (useful for composed
        // forenames).
        name append_name(culture culture)
        {
            _internal_container.push_back(
                ng::instance().solver(true, _gender, culture));
            _internal_string.append(L" ").append(_internal_container.back());

            return *this;
        }

        // Method to append a surname to the generated name.
        name append_surname()
        {
            _internal_container.push_back(
                ng::instance().solver(false, _gender, _culture));
            _internal_string.append(L" ").append(_internal_container.back());

            return *this;
        }

        // Method to append a surname to the generated name.
        name append_surname(culture culture)
        {
            _internal_container.push_back(
                (ng::instance().solver(false, _gender, culture)));
            _internal_string.append(L" ").append(_internal_container.back());

            return *this;
        }

        // Operator wstring to allow for implicit conversion to string.
        operator std::wstring() const // NOLINT(hicpp-explicit-conversions)
        {
            return _internal_string;
        }

        //Added by lucan8, converting wstring to string
        operator std :: string() const{
            using convert_type = std :: codecvt_utf8<wchar_t>;
            std :: wstring_convert<convert_type, wchar_t> converter;
            return converter.to_bytes(_internal_string);
        }

        //Added by lucan8, a way to retrive a pair of the name and nationality of the person
        operator std :: pair<std :: wstring, std :: string>() const{
            return std :: make_pair(_internal_string, ng :: instance().culture_to_string(_culture));
        }

        // Operator wstring list to allow for implicit conversion to list. //
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        operator std::list<std::wstring>() const
        {
            return _internal_container;
        }

        // Operator ostream streaming internal string.
        friend std::wostream& operator<<(std::wostream& wos, const name& name)
        {
            wos << name._internal_string;
            return wos;
        }

      private:
        // Private constructor, this is mostly a helper class to the name
        // generator, not the intended API.
        name(std::wstring name_str, const gender gender, const culture culture)
            : _internal_string(std::move(name_str)), _gender(gender),
              _culture(culture), _internal_container({_internal_string})
        {
        }

        // Internal string containing all name and appended surnames.
        std::wstring _internal_string;

        // Internal container with name and surnames.
        std::list<std::wstring> _internal_container;

        // Internal indicator of original gender of the first generated name.
        gender _gender;

        // Internal indicator of original culture of the first generated name.
        culture _culture;

        // Allows name generator to construct names.
        friend class ng;
    };

    // Thread safe access to name generator singleton.
    static ng& instance()
    {
        static ng instance;
        return instance;
    }

    //Added by lucan8, retriving nationality as string from map with key of type enum "culture"
    std :: string culture_to_string(culture _culture)
    {
        try{
            return _culture_string.at(_culture);
        } catch(std :: out_of_range& e){
            std :: cerr << "Error(culture_to_string): culture not found(" << (int)_culture << ")\n";
            return "";
        }
    }

    // Translates ISO 3166 2-letter country code to internal culture enum,
    // unknown or unsupported code will be translated as any.
    static culture to_culture(const std::wstring& country_code)
    {
        // Maps ISO 3166 2-letter country codes to internal culture enum.
        static const std::map<std::wstring, culture> country_code_map = {
            {L"ar", culture::argentinian}, {L"us", culture::american},
            {L"au", culture::australian},  {L"br", culture::brazilian},
            {L"gb", culture::british},     {L"bg", culture::bulgarian},
            {L"ca", culture::canadian},    {L"cn", culture::chinese},
            {L"dk", culture::danish},      {L"fi", culture::finnish},
            {L"fr", culture::french},      {L"de", culture::german},
            {L"kz", culture::kazakh},      {L"mx", culture::mexican},
            {L"no", culture::norwegian},   {L"pl", culture::polish},
            {L"pt", culture::portuguese},  {L"ru", culture::russian},
            {L"es", culture::spanish},     {L"se", culture::swedish},
            {L"tr", culture::turkish},     {L"ua", culture::ukrainian}};

        return (country_code_map.find(country_code) != country_code_map.end())
                   ? country_code_map.at(country_code)
                   : culture::any;
    }

    // Translates possible gender strings to gender enum.
    static gender to_gender(const std::wstring& gender_string)
    {
        static const std::map<std::wstring, gender> gender_map = {
            {L"m", gender::m},
            {L"f", gender::f},
            {L"male", gender::m},
            {L"female", gender::f}};

        return (gender_map.find(gender_string) != gender_map.end())
                   ? gender_map.at(gender_string)
                   : gender::any;
    }

    // Generates a first name based on requested gender and culture.
    [[nodiscard]] name get_name(gender gender = gender::any,
                                culture culture = culture::any) const
    {
        return solver(true, gender, culture);
    };

    //Moved the following lines of code here for convinience(lucan8)
    culture get_rand_culture() const{
            return static_cast<culture>
                                        (effolkronium::random_thread_local::get<std::size_t>
                                            (0, static_cast<std::size_t>(culture::any) - 1)
                                        );
    }

    // Generates a surname based on requested culture.
    [[nodiscard]] name get_surname(culture culture = culture::any) const
    {
        return solver(false, gender::any, culture);
    };

    // Try loading every possible names file from the received resource path.
    void load(const std::filesystem::path& resource_path)
    {
        if (loaded == false && std::filesystem::exists(resource_path) &&
            std::filesystem::is_directory(resource_path))
        {
            uint16_t nationality_index = 0;
            for (const auto& dir_entry :
                 std::filesystem::recursive_directory_iterator(resource_path))
            {
                if (dir_entry.is_regular_file() &&
                    (dir_entry.path().extension() == ".names"))
                {
                    loaded = true;
                    parse_file(dir_entry);
                }
                //Added by lucan8, retriving nationality strings in a map
                else{
                    std :: string nationality = dir_entry.path().string();
                    nationality = nationality.substr(nationality.find_last_of('\\') + 1);
                    _culture_string[(culture)nationality_index ++] = nationality;
                }
            }
        }
    };

  private:
    // A pointer to a container of names.
    using name_container_ptr = std::shared_ptr<std::vector<std::wstring>>;

    // Default folder to look for names and surnames resources.
    static const inline std::filesystem::path _default_resources_path =
        "resources";

    // Maps for accessing male name through culture.
    std::map<culture, name_container_ptr> _culture_indexed_m_names;

    //Added by lucan8, a way to get the string nationality using the enum culture
    std :: unordered_map<culture, std :: string> _culture_string;

    // Maps for accessing female name through culture.
    std::map<culture, name_container_ptr> _culture_indexed_f_names;

    // Maps for accessing surname through culture.
    std::map<culture, name_container_ptr> _culture_indexed_surnames;

    //Added by lucan8, a way to determine whether we already loaded everything
    bool loaded = false;

    // Initialize random generator, no complicated processes.
    ng()
    {
        load(_default_resources_path);
    };

    // We don't manage any resource, all should gracefully deallocate by itself.
    ~ng() = default;

    // Contains logic to retrieve a random name/surname from the containers.
    [[nodiscard]] name solver(const bool is_name, gender requested_gender,
                              culture requested_culture) const
    {
        // Randomly select a culture if necessary.
        requested_culture =
            (requested_culture == culture::any) ? get_rand_culture(): requested_culture;

        // Randomly select gender if necessary.
        requested_gender =
            (requested_gender == gender::any)
                ? static_cast<gender>(
                      effolkronium::random_thread_local::get<std::size_t>(
                          0, static_cast<std::size_t>(gender::any) - 1))
                : requested_gender;

        // Decides what container it will access.
        if (is_name)
        {
            // Decides what container it will access.
            switch (requested_gender)
            {
            case gender::m: {
                // Randomly select a name of the requested gender and culture.
                return {*effolkronium::random_thread_local::get(
                            *_culture_indexed_m_names.at(requested_culture)),
                        requested_gender, requested_culture};
            }
            case gender::f: {
                // Randomly select a name of the requested gender and culture.
                return {*effolkronium::random_thread_local::get(
                            *_culture_indexed_f_names.at(requested_culture)),
                        requested_gender, requested_culture};
            }
            default: {
                // Randomly select a name of the requested gender and culture.
                return {*effolkronium::random_thread_local::get(
                            *_culture_indexed_m_names.at(requested_culture)),
                        requested_gender, requested_culture};
            }
            }
        }
        else
        {
            // Randomly select a surname of the requested culture.
            return {*effolkronium::random_thread_local::get(
                        *_culture_indexed_surnames.at(requested_culture)),
                    requested_gender, requested_culture};
        }
    };

    // Try parsing the names file and index it into our container.
    void parse_file(const std::filesystem::path& file)
    {
        //std :: cout << file << '\n';
        // Expected names file format is <ISO 3166 2-letter country code>,
        // <male|female|surname>, list of names.
        std::wifstream tentative_file(file);

        // If managed to open the file proceed.
        if (tentative_file.is_open())
        {
            // Expected delimiter character.
            const wchar_t delimiter('\n');

            // Line being read from the file.
            std::wstring file_line;

            // Culture read from file header.
            culture culture_read = culture::any;

            // Gender read from file header.
            gender gender_read = gender::any;

            // List of parsed names.
            name_container_ptr names_read =
                std::make_shared<std::vector<std::wstring>>();

            // Retrieves culture from file being read.
            if (std::getline(tentative_file, file_line, delimiter))
            {
                culture_read = to_culture(file_line);
            }

            // We can't continue without a valid culture.
            if (culture_read != culture::any)
            {
                // Retrieves gender, if unable to parse a gender it should be a
                // surname file.
                if (std::getline(tentative_file, file_line, delimiter))
                {
                    gender_read = to_gender(file_line);
                }

                // Retrieves list of names.
                while (std::getline(tentative_file, file_line, delimiter))
                {
                    names_read->push_back(file_line);
                }

                // Correctly index container.
                switch (gender_read)
                {
                case gender::m: {
                    _culture_indexed_m_names.emplace(culture_read, names_read);
                    break;
                }
                case gender::f: {
                    _culture_indexed_f_names.emplace(culture_read, names_read);
                    break;
                }
                case gender::any:
                default: {
                    _culture_indexed_surnames.emplace(culture_read, names_read);
                    break;
                }
                }
            }
        }
    }
};
} // namespace dasmig