#include <Tester.h>

#include <Version.h>

namespace Glory::Test
{
	class VersionTest : public Utils::Tester
	{
	public:
		VersionTest();
		virtual ~VersionTest();

	private:
		void Constructor();
		void Move();
		void Copy();
		void Parse();
		void IsValid();
		void VersionString();
		void Compare();

		void Initialize();
		void Cleanup();
	};

	VersionTest::VersionTest()
	{
		AddTests({
			&VersionTest::Constructor,
			&VersionTest::Move,
			&VersionTest::Copy,
			&VersionTest::Parse,
			&VersionTest::IsValid,
			&VersionTest::VersionString,
			&VersionTest::Compare,
			});
	}

	VersionTest::~VersionTest()
	{
	}

	void VersionTest::Constructor()
	{
		Version zeroVersion;
		GLORY_TEST_COMPARE(zeroVersion[0], 0);
		GLORY_TEST_COMPARE(zeroVersion[1], 0);
		GLORY_TEST_COMPARE(zeroVersion[2], 0);
		GLORY_TEST_COMPARE(zeroVersion[3], 0);

		Version version{ 4, 2, 0, 5 };
		GLORY_TEST_COMPARE(version[0], 4);
		GLORY_TEST_COMPARE(version[1], 2);
		GLORY_TEST_COMPARE(version[2], 0);
		GLORY_TEST_COMPARE(version[3], 5);
	}

#pragma warning ( push )
#pragma warning ( disable : 26800 )
	void VersionTest::Move()
	{
		Version version{ 6, 9, 0, 3 };
		Version movedVersion = std::move(version);

		GLORY_TEST_COMPARE(version[0], 0);
		GLORY_TEST_COMPARE(version[1], 0);
		GLORY_TEST_COMPARE(version[2], 0);
		GLORY_TEST_COMPARE(version[3], 0);

		GLORY_TEST_COMPARE(movedVersion[0], 6);
		GLORY_TEST_COMPARE(movedVersion[1], 9);
		GLORY_TEST_COMPARE(movedVersion[2], 0);
		GLORY_TEST_COMPARE(movedVersion[3], 3);

		Version movedVersion2{ std::move(movedVersion) };
		GLORY_TEST_COMPARE(movedVersion[0], 0);
		GLORY_TEST_COMPARE(movedVersion[1], 0);
		GLORY_TEST_COMPARE(movedVersion[2], 0);
		GLORY_TEST_COMPARE(movedVersion[3], 0);

		GLORY_TEST_COMPARE(movedVersion2[0], 6);
		GLORY_TEST_COMPARE(movedVersion2[1], 9);
		GLORY_TEST_COMPARE(movedVersion2[2], 0);
		GLORY_TEST_COMPARE(movedVersion2[3], 3);
	}
#pragma warning ( pop )

	void VersionTest::Copy()
	{
		Version version{ 2, 0, 1, 4 };
		Version copiedVersion{ version };
		Version copiedVersion2 = copiedVersion;

		GLORY_TEST_COMPARE(version[0], 2);
		GLORY_TEST_COMPARE(version[1], 0);
		GLORY_TEST_COMPARE(version[2], 1);
		GLORY_TEST_COMPARE(version[3], 4);

		GLORY_TEST_COMPARE(copiedVersion[0], 2);
		GLORY_TEST_COMPARE(copiedVersion[1], 0);
		GLORY_TEST_COMPARE(copiedVersion[2], 1);
		GLORY_TEST_COMPARE(copiedVersion[3], 4);

		GLORY_TEST_COMPARE(copiedVersion2[0], 2);
		GLORY_TEST_COMPARE(copiedVersion2[1], 0);
		GLORY_TEST_COMPARE(copiedVersion2[2], 1);
		GLORY_TEST_COMPARE(copiedVersion2[3], 4);
	}
	void VersionTest::Parse()
	{
		const char* versionString1 = "0.1.6";
		const char* versionString2 = "2.5.0.rc-7";
		const char* versionStringIncorrectRC = "6.9.5.rc4";
		const char* incompleteVersionString1 = "7";
		const char* incompleteVersionString2 = "8.9";
		const char* incorrectVersionString = "a.b.c";

		Version version1 = Version::Parse(versionString1);
		Version version2 = Version::Parse(versionString2);
		Version versionIncorrectRC = Version::Parse(versionStringIncorrectRC);

		GLORY_TEST_COMPARE(version1[0], 0);
		GLORY_TEST_COMPARE(version1[1], 1);
		GLORY_TEST_COMPARE(version1[2], 6);
		GLORY_TEST_COMPARE(version1[3], 0);

		GLORY_TEST_COMPARE(version2[0], 2);
		GLORY_TEST_COMPARE(version2[1], 5);
		GLORY_TEST_COMPARE(version2[2], 0);
		GLORY_TEST_COMPARE(version2[3], 7);

		GLORY_TEST_COMPARE(versionIncorrectRC[0], 6);
		GLORY_TEST_COMPARE(versionIncorrectRC[1], 9);
		GLORY_TEST_COMPARE(versionIncorrectRC[2], 5);
		GLORY_TEST_COMPARE(versionIncorrectRC[3], 0);

		GLORY_TEST_THROW(Version incompleteVersion1 = Version::Parse(incompleteVersionString1), std::exception);
		GLORY_TEST_THROW(Version incompleteVersion2 = Version::Parse(incompleteVersionString1), std::exception);

		Version incorrectVersion = Version::Parse(incorrectVersionString);
		GLORY_TEST_COMPARE(incorrectVersion[0], 0);
		GLORY_TEST_COMPARE(incorrectVersion[1], 0);
		GLORY_TEST_COMPARE(incorrectVersion[2], 0);
		GLORY_TEST_COMPARE(incorrectVersion[3], 0);
	}

	void VersionTest::IsValid()
	{
		Version validVersion1{ 1, 0, 0, 0 };
		Version validVersion2{ 0, 1, 0, 0 };
		Version validVersion3{ 0, 0, 1, 0 };
		Version invalidVersion1{ 0, 0, 0, 0 };
		Version invalidVersion2{ 0, 0, 0, 1 };

		GLORY_TEST_VERIFY(validVersion1.IsValid());
		GLORY_TEST_VERIFY(validVersion2.IsValid());
		GLORY_TEST_VERIFY(validVersion3.IsValid());

		GLORY_TEST_FAIL(invalidVersion1.IsValid());
		GLORY_TEST_FAIL(invalidVersion2.IsValid());
	}

	void VersionTest::VersionString()
	{
		Version version1{ 9, 8, 7, 0 };
		Version version2{ 4, 3, 8, 9 };

		std::string version1Str, version2Str;
		version1.GetVersionString(version1Str);
		version2.GetVersionString(version2Str);
		GLORY_TEST_COMPARE(version1Str, std::string("9.8.7"));
		GLORY_TEST_COMPARE(version2Str, std::string("4.3.8.rc-9"));
	}

	void VersionTest::Compare()
	{
		Version version1{ 1, 0, 0, 0 };
		Version version1Copy{ version1 };
		Version version01{ 0, 1, 0, 0 };
		Version version11{ 1, 1, 0, 0 };
		Version version20{ 2, 0, 0, 0 };
		Version version1001{ 1, 0, 0, 1 };
		Version version1002{ 1, 0, 0, 2 };

		GLORY_TEST_COMPARE(Version::Compare(version1, version1Copy), 0);
		GLORY_TEST_COMPARE(Version::Compare(version1, version01), 1);
		GLORY_TEST_COMPARE(Version::Compare(version1, version11), -1);
		GLORY_TEST_COMPARE(Version::Compare(version1, version20), -1);
		GLORY_TEST_COMPARE(Version::Compare(version1, version1001), 1);
		GLORY_TEST_COMPARE(Version::Compare(version1, version1002), 1);
		GLORY_TEST_COMPARE(Version::Compare(version1001, version1002), -1);
		GLORY_TEST_COMPARE(Version::Compare(version1, version1001, true), 0);
		GLORY_TEST_COMPARE(Version::Compare(version20, version1001), 1);
	}
}

GLORY_TEST_MAIN(Glory::Test::VersionTest)
