#include "InventoryItemFragment_Stackable.h"
#include "HorrorDRB5_8/InventorySystem/Items/ItemDefinition.h" 

int32 UInventoryItemFragment_Stackable::GetItemMaxStackCount(TSubclassOf<UItemDefinition> ItemDefinition) 
{
	if (ItemDefinition)
	{
		// Cerchiamo il frammento passando la StatickClass in cui ci troviamo cioè stackable
		const UInventoryItemFragment* FoundFragment = UItemDefinition::FindFragmentByClass(ItemDefinition, StaticClass());
		
		// controllo che il frammento sia stackable
		if (const UInventoryItemFragment_Stackable* StackableFragment = Cast<UInventoryItemFragment_Stackable>(FoundFragment))
		{
			if (StackableFragment->MaxStackCount > 1)
			{
				return StackableFragment->MaxStackCount;
			}
		}
	}
	
	return 1;
}