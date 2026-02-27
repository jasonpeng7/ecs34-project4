#include <gtest/gtest.h>
#include "StringUtils.h"

// done edge case
TEST(StringUtilsTest, SliceTest){
    EXPECT_EQ(StringUtils::Slice("Slice String Test", 0, 3), std::string("Sli"));
    EXPECT_EQ(StringUtils::Slice("Slice String Test", 3, 3), std::string(""));
    EXPECT_EQ(StringUtils::Slice("Slice String Test", 3, 4), std::string("c"));
    EXPECT_EQ(StringUtils::Slice("Slice String Test", 0, 0), std::string("Slice String Test"));
    EXPECT_EQ(StringUtils::Slice("Slice String Test", 4, 8), std::string("e St"));
    EXPECT_EQ(StringUtils::Slice("hello", 0, 4), std::string("hell"));
    EXPECT_EQ(StringUtils::Slice("hello", 0, 3), std::string("hel"));

    // start = end
    EXPECT_EQ(StringUtils::Slice("hello", 4, 4), std::string(""));

    EXPECT_EQ(StringUtils::Slice("hello", 3, 4), std::string("l"));
    EXPECT_EQ(StringUtils::Slice("hello", 3, 1), std::string(""));
    // handling out of bounds?
    EXPECT_EQ(StringUtils::Slice("hello", 2, 10), std::string("llo"));
    EXPECT_EQ(StringUtils::Slice("hello", 4, 100), std::string("o"));
    EXPECT_EQ(StringUtils::Slice("hello", 10, 11), std::string(""));

    // no specify end
    EXPECT_EQ(StringUtils::Slice("hello", 2), std::string("llo"));
    EXPECT_EQ(StringUtils::Slice("hello", 0), std::string("hello"));

    // empty string
    EXPECT_EQ(StringUtils::Slice("", 0, 0), std::string(""));
    EXPECT_EQ(StringUtils::Slice("", 1, 3), std::string(""));
    EXPECT_EQ(StringUtils::Slice("", 0), std::string(""));
    EXPECT_EQ(StringUtils::Slice("A", 0, 0), std::string("A"));

    EXPECT_EQ(StringUtils::Slice("hello", -5), std::string("hello"));
    EXPECT_EQ(StringUtils::Slice("hello", -10), std::string("hello"));

    // double neagtive
    EXPECT_EQ(StringUtils::Slice("helloooo", -4, -1), std::string("ooo"));
    EXPECT_EQ(StringUtils::Slice("helloooo", -100, -100), std::string(""));
    EXPECT_EQ(StringUtils::Slice("helloooo", -9), std::string("helloooo"));
    EXPECT_EQ(StringUtils::Slice("helloooo", -1, -4), std::string(""));
}

// done edge case
TEST(StringUtilsTest, Capitalize){
    EXPECT_EQ(StringUtils::Capitalize(""), std::string(""));
    EXPECT_EQ(StringUtils::Capitalize("123"), std::string("123"));
    EXPECT_EQ(StringUtils::Capitalize("hello"), std::string("Hello"));
    EXPECT_EQ(StringUtils::Capitalize("z"), std::string("Z"));
    EXPECT_EQ(StringUtils::Capitalize("Zebra"), std::string("Zebra"));
    EXPECT_EQ(StringUtils::Capitalize("loLLLL"), std::string("LoLLLL"));
}

// done edge case
TEST(StringUtilsTest, Upper){
    EXPECT_EQ(StringUtils::Upper(""), std::string(""));
    EXPECT_EQ(StringUtils::Upper("2183jhiuhsa!!!"), std::string("2183JHIUHSA!!!"));
    EXPECT_EQ(StringUtils::Upper("123"), std::string("123"));
    EXPECT_EQ(StringUtils::Upper("Jason"), std::string("JASON"));
    EXPECT_EQ(StringUtils::Upper("is"), std::string("IS"));
    EXPECT_EQ(StringUtils::Upper("aweSomE"), std::string("AWESOME"));
    EXPECT_EQ(StringUtils::Upper("haAHaha"), std::string("HAAHAHA"));
}

// done edge case
TEST(StringUtilsTest, Lower){
    EXPECT_EQ(StringUtils::Lower(""), std::string(""));
    EXPECT_EQ(StringUtils::Lower("123"), std::string("123"));
    EXPECT_EQ(StringUtils::Lower("UIY!!!!ER"), std::string("uiy!!!!er"));
    EXPECT_EQ(StringUtils::Lower("Jason"), std::string("jason"));
    EXPECT_EQ(StringUtils::Lower("Is"), std::string("is"));
    EXPECT_EQ(StringUtils::Lower("AWESomE"), std::string("awesome"));
    EXPECT_EQ(StringUtils::Lower("hAhA"), std::string("haha"));
}

// done edge case
TEST(StringUtilsTest, LStrip){
    EXPECT_EQ(StringUtils::LStrip("    4 spaces leading."), std::string("4 spaces leading."));
    EXPECT_EQ(StringUtils::LStrip("0 spaces leading."), std::string("0 spaces leading."));
    EXPECT_EQ(StringUtils::LStrip("0         spaces leading but mid spacing."), std::string("0         spaces leading but mid spacing."));
    EXPECT_EQ(StringUtils::LStrip("\t\t\tHello"), std::string("Hello"));
    EXPECT_EQ(StringUtils::LStrip("\t\t\tHello\t\t\t"), std::string("Hello\t\t\t"));
}

// done edge case
TEST(StringUtilsTest, RStrip){
    EXPECT_EQ(StringUtils::RStrip("4 spaces trailing.    "), std::string("4 spaces trailing."));
    EXPECT_EQ(StringUtils::RStrip("0 spaces trailing."), std::string("0 spaces trailing."));
    EXPECT_EQ(StringUtils::RStrip("0 spaces trailing but       mid      spacing."), std::string("0 spaces trailing but       mid      spacing."));
    EXPECT_EQ(StringUtils::RStrip("Hello\t\t\t"), std::string("Hello"));
    EXPECT_EQ(StringUtils::RStrip("\t\t\tHello\t\t\t"), std::string("\t\t\tHello"));
}

// done edge case
TEST(StringUtilsTest, Strip){
    EXPECT_EQ(StringUtils::Strip("   3 spaces leading and 5 spaces trailing.     "), std::string("3 spaces leading and 5 spaces trailing."));
    EXPECT_EQ(StringUtils::Strip("perfectly fine sentence"), std::string("perfectly fine sentence"));
    EXPECT_EQ(StringUtils::Strip("1 trailing. "), std::string("1 trailing."));
    EXPECT_EQ(StringUtils::Strip("10 trailing.          "), std::string("10 trailing."));
    EXPECT_EQ(StringUtils::Strip("                   20 leading."), std::string("20 leading."));
    EXPECT_EQ(StringUtils::Strip("random          spaces             everywhere but no           leading or          trailing."), std::string("random          spaces             everywhere but no           leading or          trailing."));

}

// done edge case
TEST(StringUtilsTest, Center){
    EXPECT_EQ(StringUtils::Center("banana", 50, '^'), std::string("^^^^^^^^^^^^^^^^^^^^^^banana^^^^^^^^^^^^^^^^^^^^^^"));
    EXPECT_EQ(StringUtils::Center("banana", 0), std::string("banana"));
    EXPECT_EQ(StringUtils::Center("banana", 4), std::string("banana"));
    EXPECT_EQ(StringUtils::Center("banana", 7), std::string(" banana"));
    EXPECT_EQ(StringUtils::Center("banana", 23), std::string("         banana        "));
    EXPECT_EQ(StringUtils::Center("banana", 8, '*'), std::string("*banana*"));
    EXPECT_EQ(StringUtils::Center("banana", 20), std::string("       banana       "));
    EXPECT_EQ(StringUtils::Center("banana", -1, 's'), std::string("banana"));
}


TEST(StringUtilsTest, LJust){
    EXPECT_EQ(StringUtils::LJust("banana", 0), std::string("banana"));
    EXPECT_EQ(StringUtils::LJust("banana", 10), std::string("banana    "));
    EXPECT_EQ(StringUtils::LJust("banana", 100, '.'), std::string("banana.............................................................................................."));
    EXPECT_EQ(StringUtils::LJust("banana", 6, '*'), std::string("banana"));
    EXPECT_EQ(StringUtils::LJust("banana", 7, 'D'), std::string("bananaD"));
    EXPECT_EQ(StringUtils::LJust("banana", 10, '*'), std::string("banana****"));
    EXPECT_EQ(StringUtils::LJust("banana", -1, 'k'), std::string("banana"));
    EXPECT_EQ(StringUtils::LJust("", 5, 'k'), std::string("kkkkk"));
    EXPECT_EQ(StringUtils::LJust("", 0, 'k'), std::string(""));
    EXPECT_EQ(StringUtils::LJust("", -3, 'k'), std::string(""));
    EXPECT_EQ(StringUtils::LJust("banana", 10, '\n'), std::string("banana\n\n\n\n"));
    EXPECT_EQ(StringUtils::LJust("banana", 10, '\t'), std::string("banana\t\t\t\t"));
    EXPECT_EQ(StringUtils::LJust("banana apple", 10, '-'), std::string("banana apple"));
    EXPECT_EQ(StringUtils::LJust("banana apple", 20, '-'), std::string("banana apple--------"));

}

TEST(StringUtilsTest, RJust){
    EXPECT_EQ(StringUtils::RJust("banana", 10, '*'), std::string("****banana"));
    EXPECT_EQ(StringUtils::RJust("banana", 0, '*'), std::string("banana"));
    EXPECT_EQ(StringUtils::RJust("banana", 7, 'a'), std::string("abanana"));
    EXPECT_EQ(StringUtils::RJust("banana", 20, '*'), std::string("**************banana"));
    EXPECT_EQ(StringUtils::RJust("banana", -1, 'l'), std::string("banana"));
    EXPECT_EQ(StringUtils::RJust("", 5, 'k'), std::string("kkkkk"));
    EXPECT_EQ(StringUtils::RJust("", 0, 'k'), std::string(""));
    EXPECT_EQ(StringUtils::RJust("", -3, 'k'), std::string(""));
    EXPECT_EQ(StringUtils::RJust("banana", 10, '\n'), std::string("\n\n\n\nbanana"));
    EXPECT_EQ(StringUtils::RJust("banana", 10, '\t'), std::string("\t\t\t\tbanana"));
    EXPECT_EQ(StringUtils::RJust("banana apple", 10, '-'), std::string("banana apple"));
    EXPECT_EQ(StringUtils::RJust("banana apple", 20, '-'), std::string("--------banana apple"));
}

// done edge case
TEST(StringUtilsTest, Replace){
    EXPECT_EQ(StringUtils::Replace("My Jason name is Jason", "Jason", "Nick"), std::string("My Nick name is Nick"));
    EXPECT_EQ(StringUtils::Replace("My name is Jason.", "n", "hello"), std::string("My helloame is Jasohello."));
    EXPECT_EQ(StringUtils::Replace("My name is Jason.", " ", ""), std::string("MynameisJason."));
    EXPECT_EQ(StringUtils::Replace("My name is            Jason.", " ", ""), std::string("MynameisJason."));
    EXPECT_EQ(StringUtils::Replace("My name is Jason", "nam", "NAM"), std::string("My NAMe is Jason"));
    EXPECT_EQ(StringUtils::Replace("My name is Jason.", "nose", "hello"), std::string("My name is Jason."));
    EXPECT_EQ(StringUtils::Replace("abc abc abc", "abc", "x"), std::string("x x x"));
    EXPECT_EQ(StringUtils::Replace("aaa", "a", "zz"),std::string("zzzzzz"));
    EXPECT_EQ(StringUtils::Replace("ababab", "ab", "x"), std::string("xxx"));
    EXPECT_EQ(StringUtils::Replace("aaaaa", "aaa", "jason"),std::string("jasonaa"));
    EXPECT_EQ(StringUtils::Replace("a a a a a a", " ", "v"), std::string("avavavavava"));
}

// done edge case
TEST(StringUtilsTest, Split){
    std::vector<std::string> expected1 = {"Hello", "my", "name", "is", "Jason"};
    std::vector<std::string> expected2 = {};
    std::vector<std::string> expected3 = {"a", "b", "c", "d", "ef"};
    std::vector<std::string> expected4 = {"", "ello my name-is-Jason"};
    std::vector<std::string> expected5 = {"Hello", "my", "is", "Jason"};
    std::vector<std::string> expected6 = {"Hello", "-my", "-name", "is", "Jason"};
    std::vector<std::string> expected7 = {"Hello", "Myname", "is", "     Jason ", " 90"};

    EXPECT_EQ(StringUtils::Split("Hello my name is Jason"), expected1);
    EXPECT_EQ(StringUtils::Split("Hello-my-name-is-Jason", "-"), expected1);
    EXPECT_EQ(StringUtils::Split("Hello---my---name--is--Jason", "--"), expected6);
    EXPECT_EQ(StringUtils::Split("Hello my name-is-Jason", "H"), expected4);
    EXPECT_EQ(StringUtils::Split("            "), expected2);
    EXPECT_EQ(StringUtils::Split("a         b   c d                ef"), expected3);
    EXPECT_EQ(StringUtils::Split("HellonamemynameisnameJason", "name"), expected5);
    EXPECT_EQ(StringUtils::Split("Hello78Myname78is78     Jason 78 90", "78"), expected7);
    EXPECT_EQ(StringUtils::Split("a-b-", "-"), std::vector<std::string>({"a", "b", ""}));
    EXPECT_EQ(StringUtils::Split("aJason-b-Jason", "Jason"), std::vector<std::string>({"a", "-b-", ""}));
    EXPECT_EQ(StringUtils::Split("hi", "hello"),std::vector<std::string>({"hi"}));
    EXPECT_EQ(StringUtils::Split("abababa", "aba"),std::vector<std::string>({"", "b", ""}));
    EXPECT_EQ(StringUtils::Split("  Hello   world  "),std::vector<std::string>({"Hello", "world"}));
}

TEST(StringUtilsTest, Join){
    std::vector<std::string> words1 = {"Hello", "I", "am", "here"};
    std::vector<std::string> words2 = {"Hello", "I", "am", "here", "and", "this", "is", "length", "9"};
    std::vector<std::string> words3 = {"Hello", "", "World"};
    std::vector<std::string> words4= {"Hello"};
    std::vector<std::string> words5= {"", "", "", "", ""};


    EXPECT_EQ(StringUtils::Join("", words1), std::string("HelloIamhere"));
    EXPECT_EQ(StringUtils::Join("dsd", words1), std::string("HellodsdIdsdamdsdhere"));
    EXPECT_EQ(StringUtils::Join("", words2), std::string("HelloIamhereandthisislength9"));
    EXPECT_EQ(StringUtils::Join(" ", words3), std::string("Hello  World"));
    EXPECT_EQ(StringUtils::Join("", words3), std::string("HelloWorld"));
    EXPECT_EQ(StringUtils::Join("*", words3), std::string("Hello**World"));
    EXPECT_EQ(StringUtils::Join(" ", words4), std::string("Hello"));

    EXPECT_EQ(StringUtils::Join("\t", words1), std::string("Hello\tI\tam\there"));
    EXPECT_EQ(StringUtils::Join("LOL", words5), std::string("LOLLOLLOLLOL"));
    EXPECT_EQ(StringUtils::Join("", words5), std::string(""));
}

TEST(StringUtilsTest, ExpandTabs){
    EXPECT_EQ(StringUtils::ExpandTabs("Hello\tmy\tname\tis\tJason", 2), std::string("Hello my  name  is  Jason"));
    EXPECT_EQ(StringUtils::ExpandTabs("\t\td", 0), std::string("d"));
    EXPECT_EQ(StringUtils::ExpandTabs("\t\td", 2), std::string("    d"));
    EXPECT_EQ(StringUtils::ExpandTabs("\t\td", 4), std::string("        d"));
    EXPECT_EQ(StringUtils::ExpandTabs("\t\td", 10), std::string("                    d"));

    EXPECT_EQ(StringUtils::ExpandTabs("a\t\td", 0), std::string("ad"));
    EXPECT_EQ(StringUtils::ExpandTabs("a\t\td", 2), std::string("a   d"));
    EXPECT_EQ(StringUtils::ExpandTabs("a\t\td", 3), std::string("a     d"));
    EXPECT_EQ(StringUtils::ExpandTabs("a\t\td", 4), std::string("a       d"));
    EXPECT_EQ(StringUtils::ExpandTabs("a\t\td", 10), std::string("a                   d"));

    EXPECT_EQ(StringUtils::ExpandTabs("\t\t\t\t", 10), std::string("                                        "));
    EXPECT_EQ(StringUtils::ExpandTabs("hello\t\t", 2), std::string("hello   "));
    EXPECT_EQ(StringUtils::ExpandTabs("hello\t\t", 5), std::string("hello          "));
    EXPECT_EQ(StringUtils::ExpandTabs("\t\thello", 3), std::string("      hello"));
}


// done
TEST(StringUtilsTest, EditDistance){
    EXPECT_EQ(StringUtils::EditDistance("sitting", "kitten"), 3);
    EXPECT_EQ(StringUtils::EditDistance("sitting", "kItten", true), 3);
    EXPECT_EQ(StringUtils::EditDistance("JASON", "jason"), 5);
    EXPECT_EQ(StringUtils::EditDistance("JASON", "jason", true), 0);
    EXPECT_EQ(StringUtils::EditDistance("", ""), 0);
    EXPECT_EQ(StringUtils::EditDistance("", "jason"), 5);
    EXPECT_EQ(StringUtils::EditDistance("jason", ""), 5);
}
