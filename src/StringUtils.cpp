#include "StringUtils.h"
#include <cctype>
#include <iostream>
#include <algorithm>

namespace StringUtils{

std::string Slice(const std::string &str, ssize_t start, ssize_t end) noexcept{
    std::string result = "";

    if(end == 0) { // nitta project 1 doc states that this means include end of string 
        end = str.length();
    }

    // handle negative edge cases
    if(start < 0) {
        // we offset by string length, since slice(-2...) of "hello" is the same as slice(3) or i.e. length - start
        start = str.length() + start;
    } 

    if(end < 0) {
        // same logic 
        end = str.length() + end;
    }

    if(start >= end) {
        return result;
    }

    for(int start_ptr = 0; start_ptr < str.length(); start_ptr++) {
        // not at start of slice yet
        if (start_ptr < start) {
            continue;
        }
        else if (start_ptr >= end)
        {
            break;
        }
        else{
            result += str[start_ptr];
        }
    }
    return result;
}

std::string Capitalize(const std::string &str) noexcept{
    std::string Temp = str;
    Temp[0] = toupper(Temp[0]);
    return Temp;
}

std::string Upper(const std::string &str) noexcept{
    std::string Temp = str;
    for(int i = 0; i < str.length(); i++) {
        Temp[i] = toupper(Temp[i]);
    }
    return Temp;
}

std::string Lower(const std::string &str) noexcept{
    std::string Temp = str;
    for(int i = 0; i < str.length(); i++) {
        Temp[i] = tolower(Temp[i]);
    }
    return Temp;
}

std::string LStrip(const std::string &str) noexcept{
    std::string res = "";
    int i = 0;
    while(isspace(str[i])) {
        i++;
        continue;
    }

    res = StringUtils::Slice(str, i, str.length());

    return res;
}

std::string RStrip(const std::string &str) noexcept{
    std::string res = "";
    int i = str.length() - 1;

    while(isspace(str[i])) {
        i--;
        continue;
    }

    res = StringUtils::Slice(str, 0, i + 1);
    return res;
}

std::string Strip(const std::string &str) noexcept{
    std::string temp = str;
    temp = StringUtils::LStrip(temp);
    temp = StringUtils::RStrip(temp);
    return temp;
}

// note to self: if odd width, leading space added first
std::string Center(const std::string &str, int width, char fill) noexcept{
    if (width <= str.length() || width < 0) {
        return str;
    }

    std::string Temp = "";
    // even 
    if(width % 2 == 0) {
        int split = (width - str.length()) / 2;
        std::string padding = "";
        for(int i = 0; i < split; i++) {
            padding += fill;
        }
        Temp = padding + str + padding;
    // odd
    } else {
        double rsplit = (width - str.length()) / 2;
        std::cout <<  "rsplit = " << rsplit << std::endl;
        // width - str length because thats the half split
        double lsplit = (width - str.length()) - rsplit;
        std::string lpadding = "";
        std::string rpadding = "";

        for(int i = 0; i < lsplit ; i++) {
            lpadding += fill;
        }
        for(int j = 0; j < rsplit ; j++) {
            rpadding += fill;
        }

        Temp = lpadding + str + rpadding;
    }
    return Temp;
}

std::string LJust(const std::string &str, int width, char fill) noexcept{
    if(width <= str.length() || width < 0) {
        return str;
    }

    std::string Temp = str;
    std::string padding = "";
    for(int i = 0; i < (width - str.length()); i++) {
        padding += fill;
    }
    return Temp + padding;
}

std::string RJust(const std::string &str, int width, char fill) noexcept{
    if(width <= str.length() || width < 0) {
        return str;
    }

    std::string Temp = str;
    std::string padding = "";
    for(int i = 0; i < (width - str.length()); i++) {
        padding += fill;
    }
    return padding + Temp;
}

std::string Replace(const std::string &str, const std::string &old, const std::string &rep) noexcept{
    // use splice to get the range of where i need to replace
    // "My name is Jason Peng" <--> "Jason" to "Nick"
    // all occurences of a word will be replaced
   
    std::string res = "";

    int i = 0;
    int left_curr = 0;
    int right_curr = 0;
    int left_old = 0;
    int right_old = 0;
    bool found = false;
    std::string curr_substr = StringUtils::Slice(str, left_curr, right_curr + 1);
    std::string match_substr = StringUtils::Slice(old, left_old, right_old + 1);

    while(i < str.length()) {
        found = false;

        // keep track of previous pointers IN CASE theres no full match
        int prev_i = i;
        int prev_right_curr = right_curr;
        
        while (curr_substr == match_substr && right_old < old.length()) {
            if(curr_substr.length() == old.length()) {
                // found match, append the replacement string, and skip past len(substr) char (reset)
                found = true;
                // std::cout << "found match for " << curr_substr << " and " << match_substr << std::endl;
                res += rep;
                left_curr = right_curr;
                right_old = 0;
                break;
            } else {
                i += 1;
                right_curr += 1;
                right_old += 1;
                curr_substr = StringUtils::Slice(str, left_curr, right_curr + 1);
                match_substr = StringUtils::Slice(old, left_old, right_old + 1);
            }
        }
        if (!found) {
            // restore if no match
            i = prev_i;
            right_curr = prev_right_curr;
            res += str[i];
        }
        left_curr = right_curr + 1;
        right_curr = left_curr;
        i += 1;
        curr_substr = StringUtils::Slice(str, left_curr, right_curr + 1);
        match_substr = StringUtils::Slice(old, left_old, right_old + 1);

    }

    return res;
}

std::vector< std::string > Split(const std::string &str, const std::string &splt) noexcept{
    std::vector<std::vector<int>> occurences = {}; // occurences for range of where split occurs stored as [a,b]
    std::vector<std::string> res = {};
    int i = 0;
    while(i < str.length()) {
        // iterate through whole string, but if we're at split char, we add the current string we have and restart
        std::string curr_word = "";
        // for space
        if(splt == "") {
            while(!isspace(str[i]) && i < str.length()) {
                    curr_word += str[i];
                    i += 1;
                }

                i += 1;
                if(curr_word != ""){
                    res.push_back(curr_word);
            }   
        } else {
            // for everything else (split can be longer than 1 char split by substr)
            // Hello-my-name-is-my-jason --> "my"
            int left = 0;
            int right = 0;
            while(i <= str.length()) {
                // if split is only 1 char 
                if(splt.length() == 1) {
                    std::string curr_char = StringUtils::Slice(str, i, i + 1);
                    while(curr_char != splt && i < str.length()) {
                        // std::cout << curr_char << std::endl;
                        right += 1;
                        i += 1;
                        curr_char = StringUtils::Slice(str, i, i + 1);
                    }
                    i += 1;
                    if(right == 0) { // ** handle edge case
                        res.push_back("");
                    } else {
                        res.push_back(StringUtils::Slice(str, left, right));
                    }
                    left = i;
                    right = left;
                } else { // intuition is to get all the range indices of where the split(s) occur
                    std::string curr_substr = StringUtils::Slice(str, left, right + 1);
                    int match_l = 0;
                    int match_r = 0;
                    std::string match_substr = StringUtils::Slice(splt, match_l, match_r + 1) ;

                    while(curr_substr == match_substr) {
                        if (curr_substr.length() == splt.length()) {
                            // found a possible split
                            occurences.push_back(std::vector<int>{left, right});
                            // reset match pointer and increment past what we just read
                            left = right + 1;
                            right = left;
                            i = left;
                            match_r = 0;
                            break;
                        }
                        match_r += 1;
                        right += 1;
                        i += 1;
                        curr_substr = StringUtils::Slice(str, left, right + 1);
                        match_substr = StringUtils::Slice(splt, match_l, match_r + 1);

                    }
                    if (match_r == 0) {  // if there was no successful match, increment, dont double count
                        left += 1;
                        right = left;
                        i += 1;
                    }
                    curr_substr = StringUtils::Slice(str, left, right + 1);
                }
            }
        }
    }
    // for(int i = 0; i < occurences.size(); i++ ){
    //     std::cout << occurences[i][0] << ", " << occurences[i][1] << std::endl;
    // }

    // go thru all the ranges and splice in between
    if(splt.length() > 1) {
        int m = 0;
        int curr = 0;
        while(m < str.length() && curr < occurences.size()) {
            int l_bound = occurences[curr][0];
            int r_bound = occurences[curr][1];
            if(l_bound == 0) { // **since nittas implementation required us to treat 0 as None, need to handle  edge case
                res.push_back("");
            } else {
                res.push_back(StringUtils::Slice(str, m, l_bound));
            }
            m = r_bound + 1;
            curr += 1;
        }
        res.push_back(StringUtils::Slice(str, m, str.length()));
    }

    return res;
}

std::string Join(const std::string &str, const std::vector< std::string > &vect) noexcept{
    std::string res = "";
    std::cout << "length of arr = " << vect.size() << std::endl;

    for(int i = 0; i < vect.size(); i++) {
        if(i < vect.size() - 1) {
            res += vect[i] + str; // if its the last word, dont add the fill string
        } else {
            res += vect[i];
        }
    }

    return res;
}

std::string ExpandTabs(const std::string &str, int tabsize) noexcept{
    // default tab size is 4 spaces
    // need to keep track of current element index to know next tabstop
    std::cout<< "start"<<std::endl;
    int curr_idx = 0;
    int res_idx = 0;
    int tab_stops_multiple = tabsize;
    int until_next_tab_stop;
    
    std::string res = "";


    while(curr_idx < str.length()) {
        if(str[curr_idx] != '\t') {
            res += str[curr_idx];
            curr_idx += 1;
            res_idx += 1;
        } else { // otherwise if this is a tab, we need to figure out how many whitespaces to add until next tab stop
            std::cout << "tab at index = " << curr_idx << std::endl;
            until_next_tab_stop = tabsize - (curr_idx % tabsize);

            if(res_idx % tabsize == 0) { // if we are already at a tab stop position in our res, just add tabsize whitespaces
                std::cout << "next tab stop at index = "<< until_next_tab_stop << std::endl;

                for(int i = 0; i < tabsize; i++) {
                    res += " ";
                }
                curr_idx += 1;
                res_idx += tabsize; // we need to keep track of our res ptr as well since everytime we add until tabstop spaces, our relative position should start there
            } else {
                std::cout << "next tab stop at index = "<< until_next_tab_stop << std::endl;

                for(int i = 0; i < until_next_tab_stop; i++) {
                    res += " ";
                }
                curr_idx += 1;
                res_idx += until_next_tab_stop;
            }
        }
    }

    return res;
}

int EditDistance(const std::string &left, const std::string &right, bool ignorecase) noexcept{
    // iterative matrix approach for levenshtein algo
    int s1 = left.length() + 1;
    int s2 = right.length() + 1;

    int* arr = new int[s1 * s2];

    // initialize new 2d matrix with 0's
    for (int i = 0; i < s1; i++) {
        for (int j = 0; j < s2; j++) {
            *(arr + i * s2 + j) = 0;
        }
    }

    // target and source prefixes can be transformed into empty string or empty source prefix by either dropping all char or inserting every char
    // which is why we initialize first row and column to 1,2,3...m/n

    // need to do pointer arithmetic since we have 1d array (acting as 2d) arr[0,1,2 ...]
    // offset = x * y + w 
    for(int i = 0; i < s1; i++) {
        // we want our first row only 
        arr[i * s2] = i;
    }

    for(int j = 0; j < s2; j++) {
        // we want first column only 
        arr[0 * s2 + j] = j;
    }
    
    // now we want to actually update every cell of our matrix
    int substitionCost = 0;
    for(int j = 1; j < s2; j++) {
        for(int i = 1; i < s1; i++) {
            // added +1 to both lengths in beginning to account for the base row and column in the levenshtein algo,
            // the real char we want to comp at are at i-1 and j-1, not i, j
            if(ignorecase) {
                std::string left_temp = "";
                std::string right_temp  = "";
                left_temp += left[i-1];
                right_temp += right[j-1];
                left_temp = StringUtils::Lower(left_temp);
                right_temp = StringUtils::Lower(right_temp);

                if(left_temp == right_temp) {
                    substitionCost = 0;
                } else {
                    substitionCost = 1;
                }
            } else {
                if(left[i - 1] == right[j - 1]) {
                    substitionCost = 0;
                } else {
                    substitionCost = 1;
                }
            }
            // dp we take into account miniumum if we DELETE, INSERT, OR REPLACE
            // every cell in matrix arr[i, j] = equiavlently maps to arr(i * s2 + j)
            arr[i * s2 + j] = std::min({arr[(i - 1) * s2 + j] + 1, arr[i * s2 + (j - 1)] + 1, arr[(i-1) * s2 + (j-1)] + substitionCost});
        }
    }

    // debug -  visualize matrix
    std::cout << "Matrix" << std::endl;
     for (int i = 0; i < s1; i++) {
        for (int j = 0; j < s2; j++) {
            std::cout << *(arr + i * s2 + j)
                 << " ";
        }
        std::cout << std::endl;
    }

    
    
    // matrix now filled with correct del/insert/replace, return the very last [s1, s2] index for answer

    // return last element
    return arr[(s1 - 1) * s2 + (s2 - 1)];

    // delete to prevent memory leak
    delete[] arr;

}

};