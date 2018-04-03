#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <AT2/UI/UI.h>
#include <memory>

using namespace AT2::UI;

const glm::uvec2 minimalTestTreeSize(100, 200);
std::shared_ptr<StackPanel> CreateTestTree()
{
	auto button2 = Button::Make("Button2");
	return StackPanel::Make("StackPanel1", Orientation::Horizontal, 
		{
			StackPanel::Make("StackPanel2", Orientation::Vertical,
			{
				Button::Make("Button1", minimalTestTreeSize), //the element with fixed size should restrict all tree size
			}),
			button2
		});
}

TEST(UI, CreatingAndUnreferencing)
{
	auto root = CreateTestTree();
	std::weak_ptr<Node> weak = root;

	ASSERT_EQ(root.use_count(), 1);
	ASSERT_FALSE(weak.expired());
	root = nullptr;
	ASSERT_TRUE(weak.expired());
}

TEST(UI, InsertingAdditionalNode)
{
	auto button2 = Button::Make("AdditionalButton");
	auto root = CreateTestTree();

	root->AddChild(button2);

	ASSERT_EQ(root.use_count(), 1);
	ASSERT_EQ(button2.use_count(), 2);
}

TEST(UI, InsertingAdditionalNodeAndRemoving)
{
	auto button2 = Button::Make("AdditionalButton");
	auto root = CreateTestTree();

	root->AddChild(button2);
	ASSERT_EQ(button2.use_count(), 2);
	root->RemoveChild(button2);
	ASSERT_EQ(button2.use_count(), 1);
}

TEST(UI, ComputeMinimalSize_ReturnsCorrectSizeForTestTree)
{
	auto root = CreateTestTree();
	
	auto computedSize = root->ComputeMinimalSize();
	ASSERT_EQ(computedSize, minimalTestTreeSize);
}

TEST(UI, ComputeMinimalSize_ReturnsCorrectSizeForTestTree_EvenAfterAdditionalButtonAdding)
{
	auto button2 = Button::Make("AdditionalButton");
	auto root = CreateTestTree();

	{
		ASSERT_EQ(root->ComputeMinimalSize(), minimalTestTreeSize);
	}
	//in that case size should not be changed because button have no minial size
	{
		root->AddChild(button2);
		ASSERT_EQ(root->ComputeMinimalSize(), minimalTestTreeSize);
	}

	//horizontal size should be doubled because panel is horizontal and we adding second element with same size
	{
		button2->SetSize(minimalTestTreeSize);
		ASSERT_EQ(root->ComputeMinimalSize(), glm::uvec2(minimalTestTreeSize.x * 2, minimalTestTreeSize.y));
	}
}