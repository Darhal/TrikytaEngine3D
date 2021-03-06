#include "BaseComponent.hpp"

TRE_NS_START

Vector<typename BaseComponent::ComponentMetaData>* BaseComponent::s_ComponentsTypes;

ComponentTypeID BaseComponent::RegisterComponentType(ComponentCreateFunction createfn, ComponentDeleteFunction freefn, uint32 size, Category category)
{
	if (s_ComponentsTypes == NULL) {
		s_ComponentsTypes = new Vector<ComponentMetaData>();
	}

	ComponentID componentID = (ComponentID) s_ComponentsTypes->Size();
	s_ComponentsTypes->EmplaceBack(ComponentMetaData(
		createfn, freefn, size)
	);

	return componentID;
}

TRE_NS_END