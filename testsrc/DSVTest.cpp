#include <gtest/gtest.h>
#include "DSVWriter.h"
#include "DSVReader.h"
#include "StringDataSink.h"
#include "StringDataSource.h"

TEST(DSVWriterReaderTest, EmptyRowTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,','); // pass data sink (write destination) into writer

    EXPECT_TRUE(DataSink->String().empty()); //check sink empty
    EXPECT_EQ(DataSink->String(),"");
    EXPECT_TRUE(Writer.WriteRow({})); // write empty row
    EXPECT_EQ(DataSink->String(),"");

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1; // if we read an empty row, doesnt convey info and therefore we can consider as empty file, so expect false
    EXPECT_FALSE(Reader.ReadRow(Row1));

    EXPECT_EQ(Row1.size(), 0);
}

TEST(DSVWriterReaderTest, MultipleEmptyLineTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,','); // pass data sink (write destination) into writer

    EXPECT_TRUE(DataSink->String().empty()); //check sink empty
    EXPECT_EQ(DataSink->String(),"");

    EXPECT_TRUE(Writer.WriteRow({""})); // write empty line, still valid row
    EXPECT_TRUE(Writer.WriteRow({""})); // write empty line, still valid row
    EXPECT_TRUE(Writer.WriteRow({""})); // write empty line, still valid row

    EXPECT_EQ(DataSink->String(),"\n\n");

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1; // we wrote a empty first row
    EXPECT_TRUE(Reader.ReadRow(Row1));
    EXPECT_EQ(Row1.size(), 1);

    std::vector<std::string> Row2; //we wrote a empty second row
    EXPECT_TRUE(Reader.ReadRow(Row2));
    EXPECT_EQ(Row2.size(), 1);

    std::vector<std::string> Row3; // last row is empty and nothing after, read false
    EXPECT_FALSE(Reader.ReadRow(Row3));
}

TEST(DSVWriterReaderTest, SingleRowTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',');

    EXPECT_TRUE(DataSink->String().empty());
    EXPECT_EQ(DataSink->String(),"");
    EXPECT_TRUE(Writer.WriteRow({"A","B","C"}));
    EXPECT_EQ(DataSink->String(),"A,B,C");

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));

    EXPECT_EQ(Row1.size(), 3);
    EXPECT_EQ(Row1[0], "A");
    EXPECT_EQ(Row1[1], "B");
    EXPECT_EQ(Row1[2], "C");

}

TEST(DSVWriterReaderTest, SingleValueTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',');

    EXPECT_TRUE(Writer.WriteRow({"abc"}));
    EXPECT_EQ(DataSink->String(), "abc"); // should single value output \n? first should not \n, but 2nd 3rd, etc. should be \n and then row so that the last line ends with EOF

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));

    EXPECT_EQ(Row1.size(), 1);
    EXPECT_EQ(Row1[0], "abc");
}

TEST(DSVWriterReaderTest, MultipleRowTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',');

    EXPECT_TRUE(DataSink->String().empty());
    EXPECT_EQ(DataSink->String(),"");
    EXPECT_TRUE(Writer.WriteRow({"A","B","C"}));
    EXPECT_TRUE(Writer.WriteRow({"D","E","F"}));
    EXPECT_TRUE(Writer.WriteRow({"G","H","I","J"}));

    EXPECT_EQ(DataSink->String(),"A,B,C\nD,E,F\nG,H,I,J");

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));

    EXPECT_EQ(Row1.size(), 3);
    EXPECT_EQ(Row1[0], "A");
    EXPECT_EQ(Row1[1], "B");
    EXPECT_EQ(Row1[2], "C");

    std::vector<std::string> Row2;
    EXPECT_TRUE(Reader.ReadRow(Row2));

    EXPECT_EQ(Row2.size(), 3);
    EXPECT_EQ(Row2[0], "D");
    EXPECT_EQ(Row2[1], "E");
    EXPECT_EQ(Row2[2], "F");

    std::vector<std::string> Row3;
    EXPECT_TRUE(Reader.ReadRow(Row3));

    EXPECT_EQ(Row3.size(), 4);
    EXPECT_EQ(Row3[0], "G");
    EXPECT_EQ(Row3[1], "H");
    EXPECT_EQ(Row3[2], "I");
    EXPECT_EQ(Row3[3], "J");
}

// edge case: value contains the delimiter

TEST(DSVWriterReaderTest, ContainsDelimiterTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',');

    EXPECT_TRUE(Writer.WriteRow({"a,b", "b,c"}));
    EXPECT_EQ(DataSink->String(), "\"a,b\",\"b,c\""); // should be "a,b", "b,c"

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));

    EXPECT_EQ(Row1.size(), 2);
    EXPECT_EQ(Row1[0], "a,b");
    EXPECT_EQ(Row1[1], "b,c");

}

// edge case: value contains double quote
TEST(DSVWriterReaderTest, ContainsDoubleQuoteTestSingleValue){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',');

    EXPECT_TRUE(Writer.WriteRow({"Jason said \"Hello\""}));
    EXPECT_EQ(DataSink->String(), "\"Jason said \"\"Hello\"\"\""); // whole value wrap with "", and replace " with ""

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));

    EXPECT_EQ(Row1.size(), 1);
    EXPECT_EQ(Row1[0], "Jason said \"Hello\"");
}

// edge case: value contains double quote, multiple rows
TEST(DSVWriterReaderTest, ContainsDoubleQuoteTestMultipleRows){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',');

    EXPECT_TRUE(Writer.WriteRow({"Jason said \"Hello\""}));
    EXPECT_TRUE(Writer.WriteRow({"Jason said \"Hello\""}));
    EXPECT_TRUE(Writer.WriteRow({"Jason said \"Hello\""}));
    EXPECT_EQ(DataSink->String(), "\"Jason said \"\"Hello\"\"\"\n\"Jason said \"\"Hello\"\"\"\n\"Jason said \"\"Hello\"\"\"");

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));
    EXPECT_EQ(Row1.size(), 1);
    EXPECT_EQ(Row1[0], "Jason said \"Hello\"");

    std::vector<std::string> Row2;
    EXPECT_TRUE(Reader.ReadRow(Row2));
    EXPECT_EQ(Row2.size(), 1);
    EXPECT_EQ(Row2[0], "Jason said \"Hello\"");

    std::vector<std::string> Row3;
    EXPECT_TRUE(Reader.ReadRow(Row3));
    EXPECT_EQ(Row3.size(), 1);
    EXPECT_EQ(Row3[0], "Jason said \"Hello\"");
}

// edge case: value contains newline, singlerow
TEST(DSVWriterReaderTest, ContainsNewlineSingleValue){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',');

    EXPECT_TRUE(Writer.WriteRow({"line1\nline2"}));
    EXPECT_EQ(DataSink->String(), "\"line1\nline2\""); // whole value must be double quoted 

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));
    EXPECT_EQ(Row1.size(), 1);
    EXPECT_EQ(Row1[0], "line1\nline2");
}

// edge case: value contains newline and delimiter mixed between rows
TEST(DSVWriterReaderTest, ContainsNewlineAndDelimiterMix){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,'+');

    EXPECT_TRUE(Writer.WriteRow({"line1\nline2+", "line8", "line9"}));
    EXPECT_TRUE(Writer.WriteRow({"line3+line4"}));
    EXPECT_TRUE(Writer.WriteRow({"line1\n"}));

    EXPECT_EQ(DataSink->String(), "\"line1\nline2+\"+line8+line9\n\"line3+line4\"\n\"line1\n\""); // whole value must be double quoted 

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, '+');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));
    EXPECT_EQ(Row1.size(), 3);
    EXPECT_EQ(Row1[0], "line1\nline2+");
    EXPECT_EQ(Row1[1], "line8");
    EXPECT_EQ(Row1[2], "line9");

    std::vector<std::string> Row2;
    EXPECT_TRUE(Reader.ReadRow(Row2));
    EXPECT_EQ(Row2.size(), 1);
    EXPECT_EQ(Row2[0], "line3+line4");

    std::vector<std::string> Row3;
    EXPECT_TRUE(Reader.ReadRow(Row3));
    EXPECT_EQ(Row3.size(), 1);
    EXPECT_EQ(Row3[0], "line1\n");
}

// edge case: value contains, multiple rows
TEST(DSVWriterReaderTest, ContainsNewlineMultipleRows){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',');

    EXPECT_TRUE(Writer.WriteRow({"line1\nline2"}));
    EXPECT_TRUE(Writer.WriteRow({"line3\nline4"}));
    EXPECT_TRUE(Writer.WriteRow({"line5\nline6"}));

    EXPECT_EQ(DataSink->String(), "\"line1\nline2\"\n\"line3\nline4\"\n\"line5\nline6\""); // whole value must be double quoted 

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));
    EXPECT_EQ(Row1.size(), 1);
    EXPECT_EQ(Row1[0], "line1\nline2");

    std::vector<std::string> Row2;
    EXPECT_TRUE(Reader.ReadRow(Row2));
    EXPECT_EQ(Row2.size(), 1);
    EXPECT_EQ(Row2[0], "line3\nline4");

    std::vector<std::string> Row3;
    EXPECT_TRUE(Reader.ReadRow(Row3));
    EXPECT_EQ(Row3.size(), 1);
    EXPECT_EQ(Row3[0], "line5\nline6");
}

// edge case: value contains delimiter, quotes, and newline
// ???? UNSURE
TEST(DSVWriterReaderTest, ComprehensiveEdgeCaseTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',');

    EXPECT_TRUE(Writer.WriteRow({"Jason, is\n \"awesome\""}));

    EXPECT_EQ(DataSink->String(), "\"Jason, is\n \"\"awesome\"\"\""); // whole value must be double quoted 
    
    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));
    EXPECT_EQ(Row1.size(), 1);
    EXPECT_EQ(Row1[0], "Jason, is\n \"awesome\"");
}

// edge case: quoteall = true

TEST(DSVWriterReaderTest, QuoteAllTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',', true);

    EXPECT_TRUE(Writer.WriteRow({"a", "b"}));

    EXPECT_EQ(DataSink->String(), "\"a\",\"b\""); 

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));
    EXPECT_EQ(Row1.size(), 2);
    EXPECT_EQ(Row1[0], "a");
    EXPECT_EQ(Row1[1], "b");
}

TEST(DSVWriterReaderTest, QuoteAllTestMultipleRows){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',', true);

    EXPECT_TRUE(Writer.WriteRow({"a", "b"}));
    EXPECT_TRUE(Writer.WriteRow({"a\n", "b+"}));
    EXPECT_TRUE(Writer.WriteRow({"a\"", "b\""}));
    EXPECT_TRUE(Writer.WriteRow({"a,", "b"}));

    EXPECT_EQ(DataSink->String(), "\"a\",\"b\"\n\"a\n\",\"b+\"\n\"a\"\"\",\"b\"\"\"\n\"a,\",\"b\""); 

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));
    EXPECT_EQ(Row1.size(), 2);
    EXPECT_EQ(Row1[0], "a");
    EXPECT_EQ(Row1[1], "b");

    std::vector<std::string> Row2;
    EXPECT_TRUE(Reader.ReadRow(Row2));
    EXPECT_EQ(Row2.size(), 2);
    EXPECT_EQ(Row2[0], "a\n");
    EXPECT_EQ(Row2[1], "b+");

    std::vector<std::string> Row3;
    EXPECT_TRUE(Reader.ReadRow(Row3));
    EXPECT_EQ(Row3.size(), 2);
    EXPECT_EQ(Row3[0], "a\"");
    EXPECT_EQ(Row3[1], "b\"");

    std::vector<std::string> Row4;
    EXPECT_TRUE(Reader.ReadRow(Row4));
    EXPECT_EQ(Row4.size(), 2);
    EXPECT_EQ(Row4[0], "a,");
    EXPECT_EQ(Row4[1], "b");
}


// edge case: quoteall = true and empty line
TEST(DSVWriterReaderTest, QuoteAllEmptyStringTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,',', true);

    EXPECT_TRUE(Writer.WriteRow({""})); // empty line

    EXPECT_EQ(DataSink->String(), "\"\""); 

    // std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    // CDSVReader Reader(DataSource, ',');

    // std::vector<std::string> Row1;
    // EXPECT_TRUE(Reader.ReadRow(Row1));
    // EXPECT_EQ(Row1.size(), 1);
    // EXPECT_EQ(Row1[0], "");
}

// edge case: delimiter is ", and treated as comma ,

TEST(DSVWriterReaderTest, DelimiterIsDoubleQuoteTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink, '"');

    EXPECT_TRUE(Writer.WriteRow({"ab","cd","ef"}));

    EXPECT_EQ(DataSink->String(), "ab,cd,ef"); 

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, '"');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));
    EXPECT_EQ(Row1.size(), 3);
    EXPECT_EQ(Row1[0], "ab");
    EXPECT_EQ(Row1[1], "cd");
    EXPECT_EQ(Row1[2], "ef");
}

TEST(DSVWriterReaderTest, DelimiterIsSomethingElseTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink, '7');

    EXPECT_TRUE(Writer.WriteRow({"ab", "cd", "ef"})); 

    EXPECT_EQ(DataSink->String(), "ab7cd7ef"); 

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, '7');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));
    EXPECT_EQ(Row1.size(), 3);
    EXPECT_EQ(Row1[0], "ab");
    EXPECT_EQ(Row1[1], "cd");
    EXPECT_EQ(Row1[2], "ef");
}

TEST(DSVReaderTest, SingleRowTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink, ',');

    EXPECT_TRUE(Writer.WriteRow({"ab", "cd", "ef"})); 
    EXPECT_EQ(DataSink->String(), "ab,cd,ef"); 

    // writer wrote into data sink (vector -> string)
    // now we have our reader load this into data source (string -> vector)
    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row;
    EXPECT_TRUE(Reader.ReadRow(Row));

    EXPECT_EQ(Row.size(), 3);
    EXPECT_EQ(Row[0], "ab");
    EXPECT_EQ(Row[1], "cd");
    EXPECT_EQ(Row[2], "ef");
}

TEST(DSVWriterReaderTest, MultipleEmptyRowTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink,','); // pass data sink (write destination) into writer

    EXPECT_TRUE(DataSink->String().empty()); //check sink empty
    EXPECT_EQ(DataSink->String(),"");
    EXPECT_TRUE(Writer.WriteRow({})); // write empty row
    EXPECT_TRUE(Writer.WriteRow({})); // write empty row
    EXPECT_TRUE(Writer.WriteRow({})); // write empty row
    EXPECT_TRUE(Writer.WriteRow({"hello"})); // write filled row
    EXPECT_EQ(DataSink->String(),"\n\n\nhello");

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));
    EXPECT_EQ(Row1.size(), 1);
    EXPECT_EQ(Row1[0], "");

    std::vector<std::string> Row2;
    EXPECT_TRUE(Reader.ReadRow(Row2));
    EXPECT_EQ(Row2.size(), 1);
    EXPECT_EQ(Row2[0], "");

    std::vector<std::string> Row3;
    EXPECT_TRUE(Reader.ReadRow(Row3));
    EXPECT_EQ(Row3.size(), 1);
    EXPECT_EQ(Row3[0], "");

    std::vector<std::string> Row4;
    EXPECT_TRUE(Reader.ReadRow(Row4));
    EXPECT_EQ(Row4.size(), 1);
    EXPECT_EQ(Row4[0], "hello");

    std::vector<std::string> Row5;
    EXPECT_FALSE(Reader.ReadRow(Row5));
    EXPECT_EQ(Row5.size(), 0);
}

TEST(DSVReaderTest, MultipleRowTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink, ',');

    EXPECT_TRUE(Writer.WriteRow({"ab", "cd", "ef"})); 
    EXPECT_TRUE(Writer.WriteRow({"12", "34", "56"})); 
    EXPECT_EQ(DataSink->String(), "ab,cd,ef\n12,34,56"); 

    // writer wrote into data sink (vector -> string)
    // now we have our reader load this into data source (string -> vector)
    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, ',');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));

    EXPECT_EQ(Row1.size(), 3);
    EXPECT_EQ(Row1[0], "ab");
    EXPECT_EQ(Row1[1], "cd");
    EXPECT_EQ(Row1[2], "ef");

    std::vector<std::string> Row2;
    EXPECT_TRUE(Reader.ReadRow(Row2));

    EXPECT_EQ(Row2.size(), 3);
    EXPECT_EQ(Row2[0], "12");
    EXPECT_EQ(Row2[1], "34");
    EXPECT_EQ(Row2[2], "56");

    // end of file, should not successfully read
    std::vector<std::string> Row3;
    EXPECT_FALSE(Reader.ReadRow(Row3));
}

TEST(DSVReaderTest, RandomTest){
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink, '.');

    EXPECT_TRUE(Writer.WriteRow({"ab", "cd", "ef", "d\n"})); 
    EXPECT_TRUE(Writer.WriteRow({"12", "m.", "\"hello\""})); 
    EXPECT_EQ(DataSink->String(), "ab.cd.ef.\"d\n\"\n12.\"m.\".\"\"\"hello\"\"\""); 

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, '.');

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));
    EXPECT_EQ(Row1.size(), 4);
    EXPECT_EQ(Row1[0], "ab");
    EXPECT_EQ(Row1[1], "cd");
    EXPECT_EQ(Row1[2], "ef");
    EXPECT_EQ(Row1[3], "d\n");

    std::vector<std::string> Row2;
    EXPECT_TRUE(Reader.ReadRow(Row2));
    EXPECT_EQ(Row2.size(), 3);
    EXPECT_EQ(Row2[0], "12");
    EXPECT_EQ(Row2[1], "m.");
    EXPECT_EQ(Row2[2], "\"hello\"");

    std::vector<std::string> Row3;
    EXPECT_FALSE(Reader.ReadRow(Row3));
}

TEST(DSVReaderTest, EndCoverageTest) {
    std::shared_ptr<CStringDataSink> DataSink = std::make_shared<CStringDataSink>();
    CDSVWriter Writer(DataSink, '.');

    EXPECT_TRUE(Writer.WriteRow({"ab", "cd", "ef", "d"})); 
    EXPECT_EQ(DataSink->String(), "ab.cd.ef.d");

    std::shared_ptr<CStringDataSource> DataSource = std::make_shared<CStringDataSource>(DataSink->String());
    CDSVReader Reader(DataSource, '.');

    EXPECT_FALSE(Reader.End());

    std::vector<std::string> Row1;
    EXPECT_TRUE(Reader.ReadRow(Row1));
    EXPECT_TRUE(Reader.End());

}
