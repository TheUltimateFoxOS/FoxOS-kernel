#pragma once

#include <interrupts/panic.h>
#include <stdint.h>
#include <renderer/font_renderer.h>
#include <interrupts/interrupt_handler.h>

namespace interrupts {
	struct interrupt_frame;

	__attribute__((interrupt)) void intr_handler_0(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_1(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_2(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_3(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_4(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_5(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_6(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_7(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_8(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_9(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_10(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_11(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_12(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_13(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_14(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_15(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_16(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_17(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_18(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_19(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_20(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_21(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_22(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_23(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_24(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_25(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_26(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_27(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_28(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_29(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_30(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_31(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_32(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_33(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_34(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_35(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_36(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_37(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_38(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_39(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_40(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_41(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_42(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_43(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_44(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_45(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_46(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_47(struct interrupt_frame* frame);
	__attribute__((interrupt)) void intr_handler_48(struct interrupt_frame* frame);

	void intr_common_handler(int intr_num, struct interrupt_frame* frame);
	
}