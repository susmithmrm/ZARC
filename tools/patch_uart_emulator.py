"""Pinned Zephyr v4.2.0 simulator-only RX ownership correction.

Do not use on physical drivers. Fail if expected source context differs.
The original upstream commit and resulting diff are archived by CI.
"""
import argparse
from pathlib import Path

def patch(source):
    start = source.index("static void uart_emul_rx_stop(")
    end = source.index("static void uart_emul_async_rx_disable_handler(", start)
    original = source[start:end]
    changed = original
    replacements = (
        ("uint8_t *rx_buf = NULL;", "uint8_t *rx_buf = NULL;\n\tuint8_t *rx_buf_next = NULL;"),
        ("rx_buf = data->rx_buf;", "rx_buf = data->rx_buf;\n\t\trx_buf_next = data->rx_buf_next;"),
        ("uart_emul_post_event(dev, &rx_buf_released_event);\n\tuart_emul_simple_event(dev, UART_RX_DISABLED);",
         "uart_emul_post_event(dev, &rx_buf_released_event);\n"
         "\tif (rx_buf_next != NULL) {\n"
         "\t\trx_buf_released_event.data.rx_buf.buf = rx_buf_next;\n"
         "\t\tuart_emul_post_event(dev, &rx_buf_released_event);\n"
         "\t}\n\tuart_emul_simple_event(dev, UART_RX_DISABLED);"),
    )
    for before, after in replacements:
        if changed.count(before) != 1 or after in changed:
            raise ValueError("unexpected UART emulator source; review upstream revision")
        changed = changed.replace(before, after, 1)
    return source[:start] + changed + source[end:]

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("zephyr_root", type=Path)
    args = parser.parse_args()
    path = args.zephyr_root / "drivers/serial/uart_emul.c"
    path.write_text(patch(path.read_text(encoding="utf-8")), encoding="utf-8")
