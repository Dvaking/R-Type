/*
** EPITECH PROJECT, 2024
** R-Type
** File description:
** i_entity
*/

#pragma once

#include <memory>

class Component;

class IEntity {
    public:
        virtual void addComponent(std::shared_ptr<Component> component) = 0;
        virtual ~IEntity() {}
	    
        template <typename T>
        std::shared_ptr<T> getComponent();
};
