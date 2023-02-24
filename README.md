# Space Rangers 2 Planetary Battles Engine

## Информация

Оригинальные исходники: https://github.com/twoweeks/MatrixGame

------

## Компиляция

Для сборки используется CMake версии выше 3.20,так что для дальнейших операций вам нужно его установить https://cmake.org/

### MSVC

1. #### Сборка солюшена

   Для сборки солюшена вы можете просто ввести данную команду: 
   `cmake -A Win32 -G "Visual Studio 17 2022" -B ./YOUR_DIRECTORY`

   Или же вы можете вопользоватся графическим интерфесом от CMake только главное не забудьте выставить что сборка именно 32 битного солюшена

2. #### Билд проекта

   После сборки солюшена, для билда вы можете использовать Visual Studio, либо также вы можете ввести команду CMake и он также всё быстро сбилдит.
   Сама команда:

   `cmake --build ./YOUR_DIRECTORY`

## Information

Original sources: https://github.com/twoweeks/MatrixGame

------

## Compilation

The compile uses CMake, version 3.20+, so you need to install it to do the following: https://cmake.org/

### MSVC

1. #### Build a solution

   To build a solution you can just type the following command: 
   `cmake -A Win32 -G "Visual Studio 17 2022" -B ./YOUR_DIRECTORY`.

   

   Or you can use CMake's GUI utility but don't forget to set it to build 32 bit versions of the software.

2. #### Build project

   After compiling the solution you can use Visual Studio to build it or you can use the CMake command to build it and it will also build it fast.
   The command itself:

   `cmake --build ./YOUR_DIRECTORY`.
