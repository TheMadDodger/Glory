#include <Tester.h>

#include <FragmentedBuffer.h>

namespace Glory::Test
{
	class FragmentedBufferTest : public Utils::Tester
	{
	public:
		FragmentedBufferTest();
		virtual ~FragmentedBufferTest();

	private:
		void Constructor();
		void Move();
		void OperatorBool();
		void AddData();
		void FindFargment();
		void AddDataMultipleFragments();
		void RemoveData();
		void AssignAndFree();

		// New tests
		void SplitAndMergeFragments();
		void EdgeFreeBoundaries();
		void UnorderedFragmentsSort();
	};

	FragmentedBufferTest::FragmentedBufferTest()
	{
		AddTests({
			&FragmentedBufferTest::Constructor,
			&FragmentedBufferTest::Move,
			&FragmentedBufferTest::OperatorBool,
			&FragmentedBufferTest::AddData,
			&FragmentedBufferTest::FindFargment,
			&FragmentedBufferTest::AddDataMultipleFragments,
			&FragmentedBufferTest::RemoveData,
			&FragmentedBufferTest::AssignAndFree,

			// New tests
			&FragmentedBufferTest::SplitAndMergeFragments,
			&FragmentedBufferTest::EdgeFreeBoundaries,
			&FragmentedBufferTest::UnorderedFragmentsSort,
		});
	}

	FragmentedBufferTest::~FragmentedBufferTest()
	{
	}

	void FragmentedBufferTest::Constructor()
	{
		FragmentedBuffer buffer1;
		FragmentedBuffer buffer2{ 250ull, nullptr };
		FragmentedBuffer buffer3{ 500ull, BufferHandle{ 1ull } };

		GLORY_TEST_COMPARE(buffer1.Size(), 0ull);
		GLORY_TEST_COMPARE(size_t(buffer1.Buffer().m_ID), 0ull);
		GLORY_TEST_COMPARE(buffer1.FragmentCount(), 0ull);

		GLORY_TEST_COMPARE(buffer2.Size(), 250ull);
		GLORY_TEST_COMPARE(size_t(buffer2.Buffer().m_ID), 0ull);
		GLORY_TEST_COMPARE(buffer2.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(buffer2.Fragment(0).first, 0ull);
		GLORY_TEST_COMPARE(buffer2.Fragment(0).second, 250ull);

		GLORY_TEST_COMPARE(buffer3.Size(), 500ull);
		GLORY_TEST_COMPARE(size_t(buffer3.Buffer().m_ID), 1ull);
		GLORY_TEST_COMPARE(buffer3.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(buffer3.Fragment(0).first, 0ull);
		GLORY_TEST_COMPARE(buffer3.Fragment(0).second, 500ull);
	}

#pragma warning ( push )
#pragma warning ( disable : 26800 )
	void FragmentedBufferTest::Move()
	{
		FragmentedBuffer buffer{ 900ull, BufferHandle{ 1ull } };
		GLORY_TEST_COMPARE(buffer.Size(), 900ull);
		GLORY_TEST_COMPARE(size_t(buffer.Buffer().m_ID), 1ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 0ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 900ull);

		FragmentedBuffer movedBuffer = std::move(buffer);
		GLORY_TEST_COMPARE(buffer.Size(), 0ull);
		GLORY_TEST_COMPARE(size_t(buffer.Buffer().m_ID), 0ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 0ull);

		GLORY_TEST_COMPARE(movedBuffer.Size(), 900ull);
		GLORY_TEST_COMPARE(size_t(movedBuffer.Buffer().m_ID), 1ull);
		GLORY_TEST_COMPARE(movedBuffer.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(movedBuffer.Fragment(0).first, 0ull);
		GLORY_TEST_COMPARE(movedBuffer.Fragment(0).second, 900ull);

		FragmentedBuffer movedBuffer2{ std::move(movedBuffer) };
		GLORY_TEST_COMPARE(buffer.Size(), 0ull);
		GLORY_TEST_COMPARE(size_t(buffer.Buffer().m_ID), 0ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 0ull);

		GLORY_TEST_COMPARE(movedBuffer.Size(), 0ull);
		GLORY_TEST_COMPARE(size_t(movedBuffer.Buffer().m_ID), 0ull);
		GLORY_TEST_COMPARE(movedBuffer.FragmentCount(), 0ull);

		GLORY_TEST_COMPARE(movedBuffer2.Size(), 900ull);
		GLORY_TEST_COMPARE(size_t(movedBuffer2.Buffer().m_ID), 1ull);
		GLORY_TEST_COMPARE(movedBuffer2.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(movedBuffer2.Fragment(0).first, 0ull);
		GLORY_TEST_COMPARE(movedBuffer2.Fragment(0).second, 900ull);
	}
#pragma warning ( pop )

	void FragmentedBufferTest::OperatorBool()
	{
		FragmentedBuffer buffer{ 900ull, BufferHandle{ 1ull } };
		GLORY_TEST_VERIFY(!!buffer);

		buffer = FragmentedBuffer{ 0ull, nullptr };
		GLORY_TEST_VERIFY(!buffer);

		buffer = FragmentedBuffer{ 500ull, nullptr };
		GLORY_TEST_VERIFY(!buffer);

		buffer = FragmentedBuffer{ 0ull, BufferHandle{ 1ull } };
		GLORY_TEST_VERIFY(!buffer);
	}

	void FragmentedBufferTest::AddData()
	{
		FragmentedBuffer buffer{ 1000ull, nullptr };
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 0ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 1000ull);

		GLORY_TEST_COMPARE(buffer.PushData(nullptr, 100ull, 0ull, nullptr), 0ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 100ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 900ull);

		GLORY_TEST_COMPARE(buffer.PushData(nullptr, 50ull, 0ull, nullptr), 100ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 150ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 850ull);

		GLORY_TEST_COMPARE(buffer.PushData(nullptr, 600ull, 0ull, nullptr), 150ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 750ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 250ull);

		GLORY_TEST_COMPARE(buffer.PushData(nullptr, 250ull, 0ull, nullptr), 750ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 0ull);
	}

	void FragmentedBufferTest::FindFargment()
	{
		std::vector<std::pair<size_t, size_t>> fragments = {
			{ 0ull, 100ull },
			{ 110ull, 50ull },
			{ 660ull, 5ull },
			{ 700ull, 600ull },
			{ 1700ull, 300ull },
		};

		const std::vector<std::pair<size_t, size_t>> fragmentsCopy = fragments;

		FragmentedBuffer buffer{ 1000ull, std::move(fragments), nullptr};

		GLORY_TEST_COMPARE(buffer.FragmentCount(), fragmentsCopy.size());
		for (size_t i = 0; i < fragmentsCopy.size(); ++i)
		{
			GLORY_TEST_COMPARE(buffer.Fragment(i).first, fragmentsCopy[i].first);
			GLORY_TEST_COMPARE(buffer.Fragment(i).second, fragmentsCopy[i].second);
		}

		size_t fragmentIndex;
		GLORY_TEST_VERIFY(buffer.FindFreeFragment(50ull, fragmentIndex));
		GLORY_TEST_COMPARE(fragmentIndex, 1ull);
		GLORY_TEST_VERIFY(buffer.FindFreeFragment(55ull, fragmentIndex));
		GLORY_TEST_COMPARE(fragmentIndex, 0ull);
		GLORY_TEST_VERIFY(buffer.FindFreeFragment(5ull, fragmentIndex));
		GLORY_TEST_COMPARE(fragmentIndex, 2ull);
		GLORY_TEST_VERIFY(buffer.FindFreeFragment(6ull, fragmentIndex));
		GLORY_TEST_COMPARE(fragmentIndex, 1ull);
		GLORY_TEST_VERIFY(buffer.FindFreeFragment(600ull, fragmentIndex));
		GLORY_TEST_COMPARE(fragmentIndex, 3ull);
		GLORY_TEST_VERIFY(buffer.FindFreeFragment(301ull, fragmentIndex));
		GLORY_TEST_COMPARE(fragmentIndex, 3ull);
		GLORY_TEST_VERIFY(buffer.FindFreeFragment(300ull, fragmentIndex));
		GLORY_TEST_COMPARE(fragmentIndex, 4ull);
		GLORY_TEST_VERIFY(!buffer.FindFreeFragment(601ull, fragmentIndex));
	}

	void FragmentedBufferTest::AddDataMultipleFragments()
	{
		std::vector<std::pair<size_t, size_t>> fragments = {
			{ 50ull, 50ull },
			{ 110ull, 25ull },
			{ 330ull, 500ull },
			{ 900ull, 250ull },
			{ 1500ull, 100ull },
		};

		const std::vector<std::pair<size_t, size_t>> fragmentsCopy = fragments;

		FragmentedBuffer buffer{ 1600ull, std::move(fragments), nullptr };

		GLORY_TEST_COMPARE(buffer.FragmentCount(), fragmentsCopy.size());
		for (size_t i = 0; i < fragmentsCopy.size(); ++i)
		{
			GLORY_TEST_COMPARE(buffer.Fragment(i).first, fragmentsCopy[i].first);
			GLORY_TEST_COMPARE(buffer.Fragment(i).second, fragmentsCopy[i].second);
		}

		size_t fragmentIndex;
		GLORY_TEST_VERIFY(buffer.FindFreeFragment(25ull, fragmentIndex));
		GLORY_TEST_COMPARE(fragmentIndex, 1ull);
		GLORY_TEST_COMPARE(buffer.PushData(nullptr, 25ull, fragmentIndex, nullptr), 110ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), fragmentsCopy.size() - 1);

		GLORY_TEST_VERIFY(buffer.FindFreeFragment(25ull, fragmentIndex));
		GLORY_TEST_COMPARE(fragmentIndex, 0ull);
		GLORY_TEST_COMPARE(buffer.PushData(nullptr, 25ull, fragmentIndex, nullptr), 50ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), fragmentsCopy.size() - 1);
		GLORY_TEST_COMPARE(buffer.PushData(nullptr, 25ull, fragmentIndex, nullptr), 75ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), fragmentsCopy.size() - 2);

		GLORY_TEST_VERIFY(buffer.FindFreeFragment(100ull, fragmentIndex));
		GLORY_TEST_COMPARE(fragmentIndex, 2ull);
		GLORY_TEST_COMPARE(buffer.PushData(nullptr, 100ull, fragmentIndex, nullptr), 1500ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), fragmentsCopy.size() - 3);
	}

	void FragmentedBufferTest::RemoveData()
	{
		std::vector<std::pair<size_t, size_t>> fragments = {
			{ 50ull, 50ull },
			{ 110ull, 25ull },
			{ 330ull, 500ull },
			{ 900ull, 250ull },
			{ 1500ull, 100ull },
		};

		const std::vector<std::pair<size_t, size_t>> dataChunks = {
			{ 0ull, 50ull },
			{ 100ull, 10ull },
			{ 135ull, 195ull },
			{ 830ull, 70ull },
			{ 1150ull, 350ull },
		};

		const std::vector<std::pair<size_t, size_t>> fragmentsCopy = fragments;

		FragmentedBuffer buffer{ 1600ull, std::move(fragments), nullptr };

		GLORY_TEST_COMPARE(buffer.FragmentCount(), fragmentsCopy.size());
		for (size_t i = 0; i < fragmentsCopy.size(); ++i)
		{
			GLORY_TEST_COMPARE(buffer.Fragment(i).first, fragmentsCopy[i].first);
			GLORY_TEST_COMPARE(buffer.Fragment(i).second, fragmentsCopy[i].second);
		}

		size_t currentSize = 0;
		for (size_t i = 0; i < dataChunks.size(); ++i)
		{
			buffer.FreeData(dataChunks[i].first, dataChunks[i].second, nullptr);

			currentSize += fragmentsCopy[i].second + dataChunks[i].second;
			GLORY_TEST_COMPARE(buffer.FragmentCount(), fragmentsCopy.size() - i);
			GLORY_TEST_COMPARE(buffer.Fragment(0).first, 0ull);
			GLORY_TEST_COMPARE(buffer.Fragment(0).second, currentSize);
		}

		GLORY_TEST_COMPARE(buffer.Size(), currentSize);
	}

	void FragmentedBufferTest::AssignAndFree()
	{
		FragmentedBuffer buffer{ 1000ull, nullptr };
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(buffer.PushData(nullptr, 1000ull, 0, nullptr), 0ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 0ull);

		buffer.FreeData(500ull, 100ull, nullptr);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 500ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 100ull);

		buffer.FreeData(100ull, 50ull, nullptr);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 2ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 100ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 50ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).first, 500ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).second, 100ull);

		buffer.FreeData(750ull, 250ull, nullptr);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 3ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 100ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 50ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).first, 500ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).second, 100ull);
		GLORY_TEST_COMPARE(buffer.Fragment(2).first, 750ull);
		GLORY_TEST_COMPARE(buffer.Fragment(2).second, 250ull);

		buffer.FreeData(150ull, 10ull, nullptr);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 3ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 100ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 60ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).first, 500ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).second, 100ull);
		GLORY_TEST_COMPARE(buffer.Fragment(2).first, 750ull);
		GLORY_TEST_COMPARE(buffer.Fragment(2).second, 250ull);

		buffer.FreeData(450ull, 50ull, nullptr);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 3ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 100ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 60ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).first, 450ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).second, 150ull);
		GLORY_TEST_COMPARE(buffer.Fragment(2).first, 750ull);
		GLORY_TEST_COMPARE(buffer.Fragment(2).second, 250ull);

		buffer.FreeData(600ull, 150ull, nullptr);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 2ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 100ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 60ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).first, 450ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).second, 550ull);
	}

	// New test: create multiple frees that should split, then merge together when overlapping/adjacent frees occur
	void FragmentedBufferTest::SplitAndMergeFragments()
	{
		FragmentedBuffer buffer{ 1000ull, nullptr };
		// Fill buffer completely
		GLORY_TEST_COMPARE(buffer.PushData(nullptr, 1000ull, 0, nullptr), 0ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 0ull);

		// Free two disjoint ranges
		buffer.FreeData(200ull, 100ull, nullptr); // 200..300
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 200ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 100ull);

		buffer.FreeData(500ull, 100ull, nullptr); // 500..600
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 2ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 200ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 100ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).first, 500ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).second, 100ull);

		// Free a middle range that bridges the two existing fragments (300..500)
		buffer.FreeData(300ull, 200ull, nullptr); // 300..500
		// After freeing 300..500, the three ranges should merge into a single fragment 200..600 (size 400)
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 200ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 400ull);
	}

	// New test: free at buffer edges and then free middle to merge into full free region
	void FragmentedBufferTest::EdgeFreeBoundaries()
	{
		FragmentedBuffer buffer{ 1000ull, nullptr };
		// Fill buffer
		GLORY_TEST_COMPARE(buffer.PushData(nullptr, 1000ull, 0, nullptr), 0ull);
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 0ull);

		// Free start and end edges
		buffer.FreeData(0ull, 100ull, nullptr);   // 0..100
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 0ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 100ull);

		buffer.FreeData(900ull, 100ull, nullptr); // 900..1000
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 2ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 0ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 100ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).first, 900ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).second, 100ull);

		// Free the entire middle region which should merge with both edge fragments into a single full-size fragment
		buffer.FreeData(100ull, 800ull, nullptr); // 100..900
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 1ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 0ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 1000ull);
		GLORY_TEST_COMPARE(buffer.Size(), 1000ull);
	}

	// New test: ensure constructor sorts unordered fragment lists (fragments should be in ascending start order)
	void FragmentedBufferTest::UnorderedFragmentsSort()
	{
		std::vector<std::pair<size_t, size_t>> fragments = {
			{ 700ull, 50ull },
			{ 0ull, 100ull },
			{ 300ull, 25ull },
		};

		FragmentedBuffer buffer{ 1000ull, std::move(fragments), nullptr };

		// Expect fragments to be sorted by start address: (0,100), (300,25), (700,50)
		GLORY_TEST_COMPARE(buffer.FragmentCount(), 3ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).first, 0ull);
		GLORY_TEST_COMPARE(buffer.Fragment(0).second, 100ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).first, 300ull);
		GLORY_TEST_COMPARE(buffer.Fragment(1).second, 25ull);
		GLORY_TEST_COMPARE(buffer.Fragment(2).first, 700ull);
		GLORY_TEST_COMPARE(buffer.Fragment(2).second, 50ull);
	}
}

GLORY_TEST_MAIN(Glory::Test::FragmentedBufferTest)