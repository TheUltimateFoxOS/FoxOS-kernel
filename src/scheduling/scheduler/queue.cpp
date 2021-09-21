#include <scheduling/scheduler/queue.h>

//#uint64_t_queue::uint64_t_queue-doc: Constructor for the uint64_t_queue class.

uint64_t_queue::uint64_t_queue() {
	this->len = 0;
	this->list[0] = 0;
}

//#uint64_t_queue::~uint64_t_queue-doc: Empty destructor for the uint64_t_queue class.
uint64_t_queue::~uint64_t_queue() {
}

//#uint64_t_queue::add-doc: Add an element to the queue.
void uint64_t_queue::add(uint64_t num) {
	this->list[this->len] = num;
	this->len++;
}

//#uint64_t_queue::next-doc: Rotate the queue to the next element.
void uint64_t_queue::next() {
	this->list[this->len] = this->list[0];
	for (int i = 0; i < this->len; i++) {
		this->list[i] = this->list[i + 1];
	}
}

//#uint64_t_queue::remove_first-doc: Remove the first element from the queue.
void uint64_t_queue::remove_first() {
	this->list[this->len] = this->list[0];

	for (int i = 0; i < this->len; i++) {
		this->list[i] = this->list[i + 1];
	}
	len--;
}