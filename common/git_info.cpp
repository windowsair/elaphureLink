#include "git_info.hpp"

#include <string>

constexpr bool el_isdigit(char c)
{
    return c >= '0' && c <= '9';
}

template <typename Iterator>
constexpr int el_git_tag_valid_check(Iterator it)
{
    using char_type = typename std::iterator_traits<Iterator>::value_type;
    auto start      = it;

    int dot_count = 0;
    while (*start) {
        char_type ch = *start++;
        if (ch == '.') {
            dot_count++;
        }
    }
    if (dot_count != 3) { // xx.xx.xx.xx
        return -1;
    }

    start     = it;
    dot_count = 0;
    while (*start) {
        char_type ch = *start++;
        if (el_isdigit(ch)) {
            continue;
        } else if (ch == '.') {
            dot_count++;
        } else {
            if (dot_count < 3) { // digit only before third dot
                return -1;
            }
        }
    }

    return 0;
}

// static_assert(el_git_tag_valid_check("01.01x.0.1-x-gabc1230") == -1);
// static_assert(el_git_tag_valid_check("1.1.1.1") == 0);
// static_assert(el_git_tag_valid_check("1.1.1.0-6-ge87f720") == 0);

static_assert(el_git_tag_valid_check(EL_GIT_TAG_INFO) == 0, "Git tag doesn't match the format requirements!");


// nothing to do
int main()
{
    return 0;
}