/*
 ============================================================================
 Name        : 05_sprite_with_transparancy.c
 Author      : Riapolov Mikhail
 Version     :
 Copyright   : Use at your own risk
 Description : Вывод маленького анимированного человечка
 на белом фоне
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if defined(__MINGW32__)
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

// Размеры окна для вывода
const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 900;

// путь к файлу с картинкой, содержащей файлы движения
static char sprite[] = "resources/adventurer.png";
static char tree[] = "resources/wooden.png";
// увеличение выводимых спрайтов (в данном примере исходно 32х32)
int scale = 4;

// Функция для инициализации библиотеки и создания окна
int initSDL();
// функция для проведения завершения работы библиотеки и освобождения всех ресурсов
void closeSDL();
// загрузка изображения
SDL_Texture* loadImage(char imagePath[]);

// Указатель на структуру, описывающую окно для вывода графики
SDL_Window* window = NULL;
// Указательн на структуру, описывающую механизм отрисовки в окне с применением аппаратного ускорения
SDL_Renderer* renderer = NULL;
// указатель на поверхность (surface), на которой мы будем рисовать и выводить её в окне
SDL_Surface* screenSurface = NULL;

// Переенные для хранения анимации персонажа
SDL_Texture* sprite_sheet;
SDL_Texture* tree_sheet;
// текущий кадр анимации
int anim_phase = 0;
// фаза анимации (бег, стояние на месте ...)
int anim_type = 0;
// количество кадров анимации для данного типа
int anim_phase_max[16] = { 13, 8, 10, 10, 10, 6, 4, 7, 13, 8, 10, 10, 10, 6, 4,
		7 };
// время с предыдущего кадра анимации
uint32_t last_frame;
// время в мс на 1 кадр
#define frame_time  60

// Структура для объекта
typedef struct things {
	int x;
	int y;
	struct things *next;
} things;

things* getLast(things *last) {
	if (last == NULL) {
		return NULL;
	}
	while (last->next) {
		last = last->next;
	}
	return last;
}

void pushThing(things* head){
	things* el = getLast(head);
	things* tmp = (things*)malloc(sizeof(things));
	tmp->x = rand() % 1500;
	tmp->y = rand() % 800;
	tmp->next = NULL;
	el->next = tmp;
}

int main(int argc, char *argv[]) {

	things* head = (things*)malloc(sizeof(things));
	head->next = NULL;
	// Инициализируем библиотеку SDL
	if (initSDL() > 1) {
		printf("Error in initialization.\n");
	} else {
		// Загружаем картирнку из файла
		sprite_sheet = loadImage(sprite);
		tree_sheet = loadImage(tree);

		SDL_Rect obj_size, screen_move;
		last_frame=SDL_GetTicks();
		screen_move.x = 500;
		screen_move.y = 500;

		int quit = 0;
		// Структура для хранения информации о событии
		SDL_Event event;
		// Основной цикл программы, выход из которого происходит при
		// появлении события выхода (от ОС или по нажатию ESC)
		while (quit == 0) {
			// Ждём появления события
			while (SDL_PollEvent(&event)) {
				// При его появлении проверяем тип
				if (event.type == SDL_QUIT)
					// Если событие выхода (сигнал от ОС, что приложение
					// нужно завершить), меняем флаг выхода
					quit = 1;
				else {
					if (event.type == SDL_KEYDOWN) {
						// Если это нажатие на клавишу клавиатуры, смотрим код
						// нажатой клавиши
						switch (event.key.keysym.sym) {
						case SDLK_UP:
							screen_move.y -= 15;
							break;
						case SDLK_DOWN:
							screen_move.y += 15;
							break;
						case SDLK_LEFT:
							anim_type = 9;
							screen_move.x -= 15;
							break;
						case SDLK_RIGHT:
							anim_type = 1;
							screen_move.x += 15;
							break;
						case SDLK_a:
							anim_type = 4;
							break;
						case SDLK_f:
							pushThing(head);
							break;
						case SDLK_ESCAPE:
							// Нажата клавиша ESC, меняем флаг выхода
							quit = 1;
							break;
						}
					}
				}
			}
			// отрисовка картинки с новым кадром анимации
			// проверка прошедшего времени:
			if ((SDL_GetTicks()- last_frame) >= frame_time) {
				anim_phase++;
				if (anim_phase >= anim_phase_max[anim_type]) anim_phase = 0;
				last_frame = SDL_GetTicks();
			}

			// часть картинки с нужным кадром анимации
			obj_size.x = 32 * anim_phase;
			obj_size.y = 32 * anim_type;
			obj_size.h = 32;
			obj_size.w = 32;
			// место для вывода кадра анимации и его увеличение
			screen_move.h = SCREEN_WIDTH / 10;
			screen_move.w = SCREEN_WIDTH / 10;

			SDL_Rect tree_size, tree_move;
			tree_size.x = 500;
			tree_size.y = 500;
			tree_size.h = 32;
			tree_size.w = 32;


			// Очищаем буфер рисования
			SDL_RenderClear(renderer);

			SDL_RenderCopy(renderer, sprite_sheet, &obj_size, &screen_move);

			things* last_el = head->next;
			while (last_el) {
				tree_move.x = last_el->x;
				tree_move.y = last_el->y;
				last_el = last_el->next;
				SDL_RenderCopy(renderer, tree_sheet, &tree_size, &tree_move);
			}
			// Выводим буфер на экран
			SDL_RenderPresent(renderer);

		}

	}
	closeSDL();
	return EXIT_SUCCESS;
}

int initSDL() {
	int success = 1;
	// Инициализируем библиотеку SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		// при появлении ошибки выводим её описание
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		success = 0;
	} else {
		// После успешной инициализации создаём окно для вывода графики
		window = SDL_CreateWindow("My game", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			// выводим ошибку, если создать окно не удалось
			printf("Window could not be created! SDL_Error: %s\n",
					SDL_GetError());
			success = 0;
		} else {
			// Инициализируем библиотеку загрузки изображений (только для JPG)
			int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
			if (!(IMG_Init(imgFlags) & imgFlags)) {
				printf("SDL_image could not initialize! SDL_image Error: %s\n",
				IMG_GetError());
				success = 0;
			}
			// Получаем поверхность для рисования
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == NULL) {
				// выводим ошибку, если создать окно не удалось
				printf("Renderer could not be created! SDL_Error: %s\n",
						SDL_GetError());
				success = 0;
			} else {
				// Задаём цвет отрисовки по умолчанию - белый
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			}
		}
	}
	return success;
}

void closeSDL() {
	// Закрываем окно и освобождаем все выделенные ему ресурсы
	SDL_DestroyWindow(window);
	// Завершаем работу библиотеки и освобождаем все выделенные ей ресурсы
	SDL_Quit();
}

SDL_Texture* loadImage(char imagePath[]) {
	printf("%s\n", imagePath);
	SDL_Texture *newTexture;
	// Загружаем изображение
	SDL_Surface* loadedSurface = IMG_Load(imagePath);
	if (loadedSurface == NULL) {
		printf("Unable to load image %s! SDL_image Error: %s\n", imagePath,
		IMG_GetError());
	} else {
		// Если успешно загрузили, преобразуем загруженную поверхность в формат экранной
		// Если этого не сделать, то при каждом выводе будет проводится преобразование
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (newTexture == NULL) {
			printf("Unable to load texture %s! SDL Error: %s\n", imagePath,
					SDL_GetError());
		} else
			// если успешно, возвращаем указатель на поверхность с изображенеим
			return newTexture;
		// Удаляем загруженную поверхность
		SDL_FreeSurface(loadedSurface);
	}
	return NULL;
}
