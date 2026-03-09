"""
Read mtr matrix data from file and collect distinct Y and X values.

Task:
- Matrix starts after [MATRIX]
- Matrix format:
  First number is Y value for the current row, then pairs of Z X values
  (maximum 6 pairs per line)
- Each row is terminated by 'E'
- Matrix is terminated by END keyword
- Print number of X values, number of Y values
- Find if one of distinct arrays is a subarray of the other
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Iterable


def is_contiguous_subarray(small: list[float], big: list[float]) -> bool:
	if not small:
		return True
	if len(small) > len(big):
		return False
	for index in range(len(big) - len(small) + 1):
		if big[index : index + len(small)] == small:
			return True
	return False


def distinct_in_order(values: Iterable[float]) -> list[float]:
	seen: set[float] = set()
	result: list[float] = []
	for value in values:
		if value in seen:
			continue
		seen.add(value)
		result.append(value)
	return result


def parse_matrix_tokens(content: str) -> list[str]:
	lines = content.splitlines()
	matrix_start = None
	for i, line in enumerate(lines):
		if line.strip().upper() == "[MATRIX]":
			matrix_start = i + 1
			break
	if matrix_start is None:
		raise ValueError("[MATRIX] section not found")

	matrix_text = "\n".join(lines[matrix_start:])
	return matrix_text.split()


def analyze_mtr_file(file_path: Path) -> tuple[list[float], list[float]]:
	tokens = parse_matrix_tokens(file_path.read_text(encoding="utf-8", errors="ignore"))

	y_all: list[float] = []
	x_all: list[float] = []

	index = 0
	token_count = len(tokens)

	while index < token_count:
		token = tokens[index]
		upper = token.upper()

		if upper == "END":
			break

		if upper == "E":
			index += 1
			continue

		try:
			y_value = float(token)
		except ValueError as exc:
			raise ValueError(f"Invalid Y value token: {token!r}") from exc

		y_all.append(y_value)
		index += 1

		while index < token_count:
			token = tokens[index]
			upper = token.upper()

			if upper == "E":
				index += 1
				break

			if upper == "END":
				return distinct_in_order(y_all), distinct_in_order(x_all)

			if index + 1 >= token_count:
				raise ValueError("Dangling Z token without X pair at end of matrix")

			z_token = tokens[index]
			x_token = tokens[index + 1]

			try:
				float(z_token)
				x_value = float(x_token)
			except ValueError as exc:
				raise ValueError(f"Invalid Z/X pair: {z_token!r} {x_token!r}") from exc

			x_all.append(x_value)
			index += 2

	return distinct_in_order(y_all), distinct_in_order(x_all)


def format_values(values: list[float], max_items: int = 12) -> str:
	if len(values) <= max_items:
		return str(values)
	head = values[: max_items // 2]
	tail = values[-(max_items // 2) :]
	return f"{head} ... {tail}"


def main() -> None:
	parser = argparse.ArgumentParser(description="Analyze [MATRIX] section in .mtr file")
	parser.add_argument("file", type=Path, help="Path to .mtr file")
	args = parser.parse_args()

	y_values, x_values = analyze_mtr_file(args.file)
	y_values = sorted(y_values)
	x_values = sorted(x_values)

	print(f"File: {args.file}")
	print(f"Number of distinct X values: {len(x_values)}")
	print(f"Number of distinct Y values: {len(y_values)}")

	x_in_y = is_contiguous_subarray(x_values, y_values)
	y_in_x = is_contiguous_subarray(y_values, x_values)

	if x_in_y:
		print("Distinct X array is a contiguous subarray of distinct Y array")
	elif y_in_x:
		print("Distinct Y array is a contiguous subarray of distinct X array")
	else:
		print("Neither distinct array is a contiguous subarray of the other")

	print(f"Distinct X values: {format_values(x_values)}")
	print(f"Distinct Y values: {format_values(y_values)}")


if __name__ == "__main__":
	main()