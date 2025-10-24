// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "FunctionLibraries/OmniTagsLibrary.h"

#if WITH_EDITOR
#include "GameplayTagsEditorModule.h"
#endif
#include "GameplayTagsManager.h"

FGameplayTagContainer UOmniTagsLibrary::GetTagsChildren(FGameplayTag Tag, bool Recursive)
{
	if(Recursive)
	{
		return UGameplayTagsManager::Get().RequestGameplayTagChildren(Tag);
	}

	/**V: RequestGameplayTagChildren does not expose the Recursive boolean so I have to manually
	 * import the code here, and on top of that, the AddChildrenTags function is private so I also
	 * have to import that code here... JUST ADD THE BOOLEAN PARAMETER, EPIC :)))*/
	FGameplayTagContainer TagContainer;
	TSharedPtr<FGameplayTagNode> GameplayTagNode = UGameplayTagsManager::Get().FindTagNode(Tag);
	if(GameplayTagNode.IsValid())
	{
		TArray< TSharedPtr<FGameplayTagNode> >& ChildrenNodes = GameplayTagNode->GetChildTagNodes();
		for(TSharedPtr<FGameplayTagNode> ChildNode : ChildrenNodes)
		{
			if(ChildNode.IsValid())
			{
				TagContainer.AddTagFast(ChildNode->GetCompleteTag());
			}
		}
	}
	return TagContainer;
}

bool UOmniTagsLibrary::DoesTagHaveChildren(FGameplayTag Tag)
{
	if(!Tag.IsValid())
	{
		return false;
	}
	
	TSharedPtr<FGameplayTagNode> GameplayTagNode = UGameplayTagsManager::Get().FindTagNode(Tag);
	if(GameplayTagNode.IsValid())
	{
		return GameplayTagNode->GetChildTagNodes().IsValidIndex(0);
	}

	return false;
}

bool UOmniTagsLibrary::IsTagChildOfTag(FGameplayTag Tag, FGameplayTag PotentialParent, bool DirectChild)
{
	return GetTagsChildren(PotentialParent, !DirectChild).HasTagExact(Tag);
}

FGameplayTag UOmniTagsLibrary::GetParentTag(FGameplayTag Tag)
{
	return UGameplayTagsManager::Get().RequestGameplayTagDirectParent(Tag);
}

bool UOmniTagsLibrary::AddNewTagToINI(FString TagName, FString TagComment, FName IniName)
{
#if WITH_EDITOR
	if(TagName.IsEmpty() || IniName.IsNone())
	{
		return false;
	}

	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI(TagName, TagComment, IniName);
	return true;
#else

	return false;
#endif
}
