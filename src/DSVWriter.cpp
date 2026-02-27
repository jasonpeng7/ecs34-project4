#include "DSVWriter.h"
#include <iostream>
#include "StringUtils.h"
#include <string.h>

struct CDSVWriter::SImplementation{
    std::shared_ptr< CDataSink > DSink;
    char DDelimiter;
    bool DQuoteAll;
    bool DHasRow;
    bool DIsQuoted;

    SImplementation(std::shared_ptr< CDataSink > sink, char delimiter, bool quoteall){
        DSink = sink;
        DDelimiter = delimiter;
        DQuoteAll = quoteall;
        DHasRow = false; // for new rows
        DIsQuoted = false; // to keep track of overlapping edge cases + whether or not to trigger quoteall Flag
    }

    ~SImplementation(){}

    bool WriteRow(const std::vector<std::string> &row){
        bool FirstValue = true;

        if(DHasRow) { // before we write the next rows (if any), write \n
            DSink->Put('\n');
        }

        // for each column, before we write anything at all, we need to make sure it goes through all edge cases.
        for(auto &Column : row){
            std::string quote = "\"";
            std::string Buf = Column;
            std::cout << "Column = " << Column << std::endl;

            // if double quote is in the value
            if (Column.find(quote) != std::string::npos) {
                // we want to quote entire string AND replace " with ""
                std::cout << "double quote present" << std::endl;

                // replace " with ""
                Buf =  StringUtils::Replace(Buf, std::string("\""), std::string("\"\""));
                
                // wrap with quotes
                Buf = quote + Buf + quote;
                // have flag if already quoted
                DIsQuoted = true;

                std::cout << Buf << std::endl;
            }

            // handle if delimiter is "
            if (DDelimiter == '\"') {
                // treat as comma
                DDelimiter = ',';
            }
             // handle if delimiter is in value or newline is in value
            else if (Column.find(DDelimiter) != std::string::npos || Column.find('\n') != std::string::npos) {
                // if delim in value, we just want to quote the value
                std::cout << "delim is in column or newline in columns" << std::endl;
                if(!DIsQuoted) {
                    Buf = quote + Buf + quote;
                }
                DIsQuoted = true;
                std::cout<< "quoted = " << Buf << std::endl;
            }

             // only put delimiter after the first value
            if(!FirstValue){
                DSink->Put(DDelimiter);
            }

            // if value has not been quoted AND the quoteall flag is TRUE, wrap with quotes
            if(!DIsQuoted && DQuoteAll) {
                std::cout << "not yet quoted, DQA flag is on, adding quotes" << std::endl;
                Buf = quote + Buf + quote;
            }

            std::vector<char> Buffer(Buf.begin(), Buf.end());

            std::cout << "Final column = " << Buf << std::endl;
            DSink->Write(Buffer);

            // reset flags
            FirstValue = false;
            DIsQuoted = false;
        }
        DHasRow = true;
        return true;
    }

};

CDSVWriter::CDSVWriter(std::shared_ptr< CDataSink > sink, char delimiter, bool quoteall){
    DImplementation = std::make_unique<SImplementation>(sink,delimiter,quoteall);
}

CDSVWriter::~CDSVWriter(){

}

bool CDSVWriter::WriteRow(const std::vector<std::string> &row){
    return DImplementation->WriteRow(row);
}
