#include "Version.h"
#include <vector>
#include <sstream>

namespace Glory
{
    Version Glory::Version::Parse(const char* str)
    {
        std::string_view strView = str;
        std::vector<std::string_view> splits;
        size_t index = 0;
        while (true)
        {
            const size_t nextIndex = strView.find(Separator, index);
            if (nextIndex == std::string_view::npos)
            {
                splits.push_back(std::string_view(strView.data() + index));
                index = nextIndex + 1;
                break;
            }
            splits.push_back(std::string_view(strView.data() + index, nextIndex - index));
            index = nextIndex + 1;
        }

        const int rc = splits.size() > 3 ? (int)std::strtol(splits[3].substr(3).data(), nullptr, 10) : 0;

        return Version((int)std::strtol(splits[0].data(), nullptr, 10),
            (int)std::strtol(splits[1].data(), nullptr, 10),
            (int)std::strtol(splits[2].data(), nullptr, 10), rc);
    }

    int Version::Compare(const Version& v1, const Version& v2)
    {
        for (size_t i = 0; i < 3; i++)
        {
            if (v1[i] == v2[i]) continue;
            if (v1[i] < v2[i]) return -1;
            if (v1[i] > v2[i]) return 1;
        }

        return v1.RC == v2.RC ? 0 : (v1.RC > v2.RC) ? -1 : 1;
    }

    int Version::operator[](size_t index) const
    {
        return v[index];
    }

    bool Version::IsValid() const
    {
        for (size_t i = 0; i < 3; i++)
        {
            if (v[i] > 0) return true;
        }
        return false;
    }

    void Version::GetVersionString(std::string& out) const
    {
        std::stringstream str;
        for (size_t i = 0; i < 3; i++)
        {
            str << v[i];
            if (i != 2) str << '.';
        }
        if (RC)
        {
            str << ".rc-" << RC;
        }
        out = str.str();
    }
}
