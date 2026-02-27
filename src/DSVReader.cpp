#include "DSVReader.h"
#include <iostream>

struct CDSVReader::SImplementation{
    std::shared_ptr< CDataSource > DSrc;
    char DDelimiter;
    bool DNewline;
    bool DIsClosedQuote;

    SImplementation(std::shared_ptr< CDataSource > src, char delimiter) {
        DSrc = src; // data source
        DDelimiter = delimiter;
        DNewline = false;
        DIsClosedQuote = false;
    };

    bool ParseValue(std::string &val) {
        // reset flags before parsing
        bool inQuotes = false;
        DIsClosedQuote = false;
        bool beginParse = false;
        DNewline = false;
        val.clear();

        while(!DSrc->End()) {
            std::cout << "current value: " << val << std::endl;
            char nextChar;
            DSrc->Peek(nextChar);

            if(inQuotes) {
                DSrc->Get(nextChar);
                beginParse = true;

                if(nextChar == '\"') {
                    std::cout << "found another quote" << std::endl;
                    // if we found another quote, it may be a " replaced with "" OR a regular ending quote
                    char nextNextChar;
                    // if the current next char = " and the next next char is also ", then we consume both, since it was OG just "
                    if(!DSrc->End() && DSrc->Peek(nextNextChar) && nextNextChar =='\"') {
                        std::cout << "quote was replaced" << std::endl;
                        DSrc->Get(nextNextChar); 
                        val += '\"';
                    } else { // true ending quote
                        std::cout << "quote was true ending" << std::endl;
                        inQuotes = false;
                        DIsClosedQuote = true;
                    }
                } else { 
                    std::cout << "in quote, regular value: " << nextChar << std::endl;
                    val += nextChar;
                }
                continue; //keep reading until we hit one of our cases
            }

            if(nextChar == DDelimiter) {
                std::cout << "delimiter found in DString" << std::endl;
                DSrc->Get(nextChar); // consume
                beginParse = true;
                return true;
            } else if (nextChar == '\n') {
                std::cout << "newline found in DString" << std::endl;
                DSrc->Get(nextChar); // consume
                DNewline = true; // set flag to true so we can successfully say we read row in ReadRow()
                beginParse = true;
                return true;
            } else if(nextChar == '\"' && !inQuotes) {
                inQuotes = true; // if we are in quotes, we dont want to consme delimiter and make new column
                std::cout << "quotes found" << std::endl;
                DSrc->Get(nextChar); // consume quote
                beginParse = true;
                continue;
            }
            else {
                DSrc->Get(nextChar);
                val += nextChar;
                beginParse = true;
            }
        }
        return beginParse;
    }

    ~SImplementation(){}

    bool End() const {
        // return true if everything has been read from DSV
        char curr_char;
        if(!DSrc->Peek(curr_char)) {
            // if we try to get curr char and it returns false, index > length so we are at EOF
            std::cout << "END OF FILE" << std::endl;
            return true;
        }
        return false;
    }

    bool ReadRow(std::vector<std::string> &row) {
        if (DDelimiter == '\"') {
            DDelimiter = ',';
        }
        std::cout << "READING ROW..." << std::endl;
        DNewline = false;
        row.clear();

        char curr_char;
        std::string curr_row = "";
        std::cout << "at eof? " << DSrc->End() << std::endl;

        while(!DSrc->End()) {
            // std::cout << "reading current char is: " << curr_char << std::endl;
            std::string nextValue;

            if(ParseValue(nextValue)) {
                std::cout << "pushing " << nextValue << " into row" << std::endl;
                if(DIsClosedQuote) {
                    std::cout << "final row is: " << nextValue << std::endl;
                    row.push_back(nextValue);
                } else {
                    std::cout << "final row is: " << nextValue << std::endl;
                    row.push_back(nextValue);
                }
            } 
            if(DNewline) {
                    std::cout << "found newline, done with row" << std::endl;
                    return true;
            }
            
            if(DSrc->End()) {
                return true;
            }
        }
        return false; // if we are at end
    }

};

CDSVReader::CDSVReader(std::shared_ptr< CDataSource > src, char delimiter) {
    DImplementation = std::make_unique<SImplementation>(src, delimiter);
}

CDSVReader::~CDSVReader() {

}

bool CDSVReader::End() const {
    return DImplementation->End();
}

bool CDSVReader::ReadRow(std::vector<std::string> &row) {
    return DImplementation->ReadRow(row);
}
