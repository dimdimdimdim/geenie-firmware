/*******************************************************************************
 * Copyright 2015 Dimitri L. <dimdimdimdim at gmx dot fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <base.h>
#include <graphics/font.h>

namespace Graphics
{

int Font::getGlyphIndex(int codePoint)
{
	if (codePoint < 32)
		return Font::GLYPH_REPLACEMENT;
	if (codePoint < 127)
		return codePoint - 32;
	if (codePoint == 0xA0) // NBSP
		return ' ' - 32;
	if (codePoint >= 0xA1 && codePoint <= 0xFF) // ¡ ... ÿ
		return codePoint - 66;
	switch (codePoint) {
	case 0x152: return 190; // Œ
	case 0x153: return 191; // œ
	case 0x160: return 192; // Š
	case 0x161: return 193; // š
	case 0x178:	return 194; // Ÿ
	case 0x17D: return 195; // Ž
	case 0x17E: return 196; // ž
	case 0x2DC: return 197; // ˜	
	case 0x391: return 'A' - 32; // capital alpha Α
	case 0x392: return 'B' - 32; // capital beta Β
	case 0x393: return 223; // capital gamma Γ
	case 0x394: case 0x2206: return 224; // capital delta Δ / increment
	case 0x395: return 'E' - 32; // capital epsilon Ε
	case 0x396: return 'Z' - 32; // capital dzêta Ζ
	case 0x397: return 'H' - 32; // capital êta Η
	case 0x398: return 225; // capital thêta Θ
	case 0x399: return 'I' - 32; // capital itoa Ι
	case 0x39A: return 'K' - 32; // capital kappa Κ
	case 0x39B: return 226; // capital lambda Λ
	case 0x39C: return 'M' - 32; // capital mu Μ
	case 0x39D: return 'N' - 32; // capital nu N
	case 0x39E: return 227; // capital xi Ξ
	case 0x39F: return 'O' - 32; // capital omicron Ο
	case 0x3A0: case 0x220F: return 228; // capital pi Π / math product
	case 0x3A1: return 'P' - 32; // capital rhô Ρ
	case 0x3A3: case 0x2211: return 229; // capital sigma Σ / math sum
	case 0x3A4: return 'T' - 32; // capital tau Τ
	case 0x3A5: return 'Y' - 32; // capital upsilon Υ
	case 0x3A6: return 230; // capital phi Φ
	case 0x3A7: return 'X' - 32; // capital khi Χ
	case 0x3A8: return 231; // capital psi Ψ
	case 0x3A9: case 0x2126: return 232; // capital omega Ω / ohm symbol
	case 0x3B1: return 233; // lowercase alpha α
	case 0x3B2: return 234; // lowercase beta β
	case 0x3B3: return 235; // lowercase gamma γ
	case 0x3B4: return 236; // lowercase delta δ
	case 0x3B5: return 237; // lowercase epsilon ε
	case 0x3B6: return 238; // lowercase dzêta ζ
	case 0x3B7: return 239; // lowercase êta η
	case 0x3B8: return 240; // lowercase theta θ
	case 0x3B9: return 241; // lowercase iota ι
	case 0x3BA: return 242; // lowercase kappa κ
	case 0x3BB: return 243; // lowercase lambda λ
	case 0x3BC: return 115; // lowercase mu μ (same as micro)
	case 0x3BD: return 244; // lowercase nu ν
	case 0x3BE: return 245; // lowercase xi ξ
	case 0x3BF: return 'o' - 32; // lowercase omicron ο
	case 0x3C0: return 246; // lowercase pi π
	case 0x3C1: return 247; // lowercase rho ρ
	case 0x3C2: return 248; // lowercase sigma final ς
	case 0x3C3: return 249; // lowercase sigma σ
	case 0x3C4: return 250; // lowercase tau τ
	case 0x3C5: return 251; // lowercase upsilon υ
	case 0x3C6: return 252; // lowercase phi φ
	case 0x3C7: return 253; // lowercase khi χ
	case 0x3C8: return 254; // lowercase psi ψ
	case 0x3C9: return 255; // lowercase omega ω
	case 0x2013: return 198; // –
	case 0x2014: return 199; // —
	case 0x2018: return 200; // ‘
	case 0x2019: return 201; // ’
	case 0x201A: return 202; // ‚
	case 0x201C: return 203; // “
	case 0x201D: return 204; // ”
	case 0x201E: return 205; // „
	case 0x2022: return 206; // •
	case 0x2026: return 208; // …
	case 0x2030: return 209; // ‰
	case 0x2039: return 210; // ‹
	case 0x203A: return 211; // ›	
	case 0x20AC: return 212; // €
	case 0x2122: return 213; // ™
	case 0x2190: return 263; // arrow left ←
	case 0x2192: return 264; // arrow right →
	case 0x2191: return 265; // arrow up ↑
	case 0x2193: return 266; // arrow down ↓
	case 0x21E7: return 267; // arrow shift ⇧
	case 0x2220: return 214; // angle ∠
	case 0x2207: return 215; // nabla ∇
	case 0x221A: return 216; // square root √
	case 0x222B: return 217; // integral ∫
	case 0x2202: return 218; // differential ∂
	case 0x2264: return 219; // less than or equal to ≤
	case 0x2265: return 220; // greater than or equal to ≥
	case 0x2260: return 221; // not equal to ≠
	case 0x221E: return 222; // infinity ∞
	case 0x25A0: return 207; // black square ■
	case 0x25B6: return 268; // black right-pointing triangle ▶
	}
	return Font::GLYPH_REPLACEMENT;
}

}
