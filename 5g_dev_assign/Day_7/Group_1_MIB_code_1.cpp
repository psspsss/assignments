/*
 * ============================================================================
 * 5G NR PBCH - Topic 1: MIB Payload Construction
 * ============================================================================
 * Reference: 3GPP TS 38.331 / TS 38.212 §7.1.1
 *
 * This program constructs the Master Information Block (MIB) payload
 * which consists of 24 bits from MIB fields + 8 additional timing bits
 * resulting in a total of 32 bits (A=32)
 *
 * Example Parameters:
 * - N_ID_cell = 276 (Physical Cell ID)
 * - SFN = 42 (System Frame Number)
 * - Half-frame = 0
 * - SSB index = 3
 * - SCS = 30 kHz (Subcarrier Spacing)
 * - k_SSB = 5
 * - L_max = 4 (Maximum number of SSBs in half frame)
 * ============================================================================
 */

#include <bitset>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

// Function to convert a decimal number to binary string with specified width
string decimalToBinary(int num, int width) {
  // Convert integer to binary representation with specified bit width
  return bitset<32>(num).to_string().substr(32 - width, width);
}

// Function to extract specific bits from a number
int extractBits(int num, int msb, int lsb) {
  // msb: most significant bit position (0-indexed from right)
  // lsb: least significant bit position
  // Example: extractBits(42, 9, 4) extracts bits 9 to 4 from number 42
  int mask = ((1 << (msb - lsb + 1)) - 1) << lsb; // Create bit mask
  return (num & mask) >> lsb; // Apply mask and shift to get extracted bits
}

int main() {
  // ========================================================================
  // STEP 1: Define Cell Parameters
  // ========================================================================

  cout << "============================================================\n";
  cout << "   5G NR PBCH - MIB Payload Construction (Topic 1)\n";
  cout << "============================================================\n\n";

  // Physical Cell ID (0-1007 in 5G NR)
  int N_ID_cell = 276;

  // System Frame Number (0-1023, 10 bits)
  int SFN = 42;

  // Half-frame indicator (0 or 1)
  int half_frame = 0;

  // SSB (SS/PBCH block) index within the half-frame
  int ssb_index = 3;

  // Subcarrier Spacing (15, 30, 60, 120, 240 kHz)
  int scs = 30; // 30 kHz

  // k_SSB: frequency domain offset
  int k_SSB = 5;

  // L_max: maximum number of SSB beams in half-frame (4, 8, or 64)
  int L_max = 4;

  // Display input parameters
  cout << "Cell Parameters:\n";
  cout << "----------------\n";
  cout << "N_ID_cell              = " << N_ID_cell << "\n";
  cout << "SFN                    = " << SFN
       << " (binary: " << decimalToBinary(SFN, 10) << ")\n";
  cout << "Half-frame             = " << half_frame << "\n";
  cout << "SSB index              = " << ssb_index << "\n";
  cout << "Subcarrier Spacing     = " << scs << " kHz\n";
  cout << "k_SSB                  = " << k_SSB
       << " (binary: " << decimalToBinary(k_SSB, 5) << ")\n";
  cout << "L_max                  = " << L_max << "\n\n";

  // ========================================================================
  // STEP 2: Construct MIB IE Fields (24 bits total)
  // ========================================================================

  cout << "MIB Information Element Fields (24 bits):\n";
  cout << "==========================================\n\n";

  // Field 1: systemFrameNumber (6 bits) - SFN[9:4]
  // Extract the 6 most significant bits of the 10-bit SFN
  int sfn_field = extractBits(SFN, 9, 4); // Bits 9 to 4 of SFN
  string sfn_field_binary = decimalToBinary(sfn_field, 6);
  cout << "1. systemFrameNumber (6 bits):\n";
  cout << "   SFN = " << SFN << " → binary: " << decimalToBinary(SFN, 10)
       << "\n";
  cout << "   Extract SFN[9:4] = " << sfn_field_binary << " (6 MSBs)\n\n";

  // Field 2: subCarrierSpacingCommon (1 bit)
  // Maps SCS to enumeration: 15kHz→0(scs15), 30kHz→1(scs30)
  int scs_common = (scs == 30) ? 1 : 0;
  string scs_common_binary = decimalToBinary(scs_common, 1);
  cout << "2. subCarrierSpacingCommon (1 bit):\n";
  cout << "   SCS = " << scs << " kHz → scs-30 → " << scs_common_binary
       << "\n\n";

  // Field 3: ssb-SubcarrierOffset (4 bits)
  // Indicates frequency offset of SSB
  string k_SSB_4bits = decimalToBinary(k_SSB, 4);
  cout << "3. ssb-SubcarrierOffset (4 bits):\n";
  cout << "   k_SSB = " << k_SSB << " → " << k_SSB_4bits << "\n\n";

  // Field 4: dmrs-TypeA-Position (1 bit)
  // DMRS position: pos2→0, pos3→1
  int dmrs_type_a_pos = 0; // pos2
  string dmrs_binary = decimalToBinary(dmrs_type_a_pos, 1);
  cout << "4. dmrs-TypeA-Position (1 bit):\n";
  cout << "   pos2 → " << dmrs_binary << "\n\n";

  // Field 5: pdcch-ConfigSIB1 (8 bits)
  // Combines controlResourceSetZero (4 bits) and searchSpaceZero (4 bits)
  int controlResourceSetZero = 10; // 4 bits
  int searchSpaceZero = 4;         // 4 bits
  int pdcch_config =
      (controlResourceSetZero << 4) | searchSpaceZero; // Combine into 8 bits
  string pdcch_config_binary = decimalToBinary(pdcch_config, 8);
  cout << "5. pdcch-ConfigSIB1 (8 bits):\n";
  cout << "   controlResourceSetZero = " << controlResourceSetZero
       << " (binary: " << decimalToBinary(controlResourceSetZero, 4) << ")\n";
  cout << "   searchSpaceZero = " << searchSpaceZero
       << " (binary: " << decimalToBinary(searchSpaceZero, 4) << ")\n";
  cout << "   Combined → " << pdcch_config_binary << "\n\n";

  // Field 6: cellBarred (1 bit)
  // Cell access barring status: notBarred→0, barred→1
  int cell_barred = 0; // notBarred
  string cell_barred_binary = decimalToBinary(cell_barred, 1);
  cout << "6. cellBarred (1 bit):\n";
  cout << "   notBarred → " << cell_barred_binary << "\n\n";

  // Field 7: intraFreqReselection (1 bit)
  // Intra-frequency cell reselection: allowed→0, notAllowed→1
  int intra_freq_resel = 0; // allowed
  string intra_freq_binary = decimalToBinary(intra_freq_resel, 1);
  cout << "7. intraFreqReselection (1 bit):\n";
  cout << "   allowed → " << intra_freq_binary << "\n\n";

  // Field 8: spare (1 bit)
  // Reserved bit, set to 0
  int spare_1bit = 0;
  string spare_binary = decimalToBinary(spare_1bit, 1);
  cout << "8. spare (1 bit):\n";
  cout << "   " << spare_binary << "\n\n";

  // Concatenate all 24 MIB bits (a0 to a23)
  string mib_24bits = sfn_field_binary + scs_common_binary + k_SSB_4bits +
                      dmrs_binary + pdcch_config_binary + cell_barred_binary +
                      intra_freq_binary + spare_binary;

  cout << "MIB = 24 bits (a0 to a23):\n";
  cout << mib_24bits << "\n\n";

  // ========================================================================
  // STEP 3: Add Additional Timing Bits (8 bits) - TS 38.212 §7.1.1
  // ========================================================================

  cout << "Additional Timing Bits (8 bits):\n";
  cout << "=================================\n\n";

  // Bit 1-2: SFN[3:2] - bits 6 and 7 of the SFN
  // Extract bits 3 and 2 from SFN (positions 6,7 in 10-bit representation)
  int sfn_bits_3_2 = extractBits(SFN, 3, 2); // Bits 3 to 2
  string sfn_3_2_binary = decimalToBinary(sfn_bits_3_2, 2);
  cout << "1. SFN[3:2] (2 bits):\n";
  cout << "   SFN = " << SFN << " → binary: " << decimalToBinary(SFN, 10)
       << "\n";
  cout << "   Extract bits [3:2] = " << sfn_3_2_binary << "\n\n";

  // Bit 3-4: SFN[1:0] - bits 8 and 9 of the SFN (LSBs)
  int sfn_bits_1_0 = extractBits(SFN, 1, 0); // Bits 1 to 0
  string sfn_1_0_binary = decimalToBinary(sfn_bits_1_0, 2);
  cout << "2. SFN[1:0] (2 bits):\n";
  cout << "   Extract bits [1:0] = " << sfn_1_0_binary << "\n\n";

  // Bit 5: Half-frame indicator (1 bit)
  string half_frame_binary = decimalToBinary(half_frame, 1);
  cout << "3. Half-frame (1 bit):\n";
  cout << "   " << half_frame_binary << "\n\n";

  // Bit 6: k_SSB MSB (5th bit) - for L_max = 4
  // Extract the MSB (most significant bit) of k_SSB (bit position 4)
  int k_SSB_msb = extractBits(k_SSB, 4, 4); // Bit 4 (MSB of 5-bit k_SSB)
  string k_SSB_msb_binary = decimalToBinary(k_SSB_msb, 1);
  cout << "4. k_SSB MSB (1 bit) - for L_max=4:\n";
  cout << "   k_SSB = " << k_SSB << " → binary: " << decimalToBinary(k_SSB, 5)
       << "\n";
  cout << "   MSB (bit 4) = " << k_SSB_msb_binary << "\n\n";

  // Bit 7-8: spare bits (2 bits)
  string spare_2bits = "00";
  cout << "5. spare[2] (2 bits):\n";
  cout << "   " << spare_2bits << "\n\n";

  // Concatenate the 8 additional timing bits (a24 to a31)
  string timing_8bits = sfn_3_2_binary + sfn_1_0_binary + half_frame_binary +
                        k_SSB_msb_binary + spare_2bits;

  cout << "Timing bits (a24 to a31):\n";
  cout << timing_8bits << "\n\n";

  // ========================================================================
  // STEP 4: Complete 32-bit Payload (A = 32 bits)
  // ========================================================================

  // Concatenate MIB (24 bits) + timing bits (8 bits) = 32 bits total
  string payload_32bits = mib_24bits + timing_8bits;

  cout << "============================================================\n";
  cout << "TOTAL PAYLOAD (A = 32 bits):\n";
  cout << "============================================================\n\n";

  cout << "A = 24 (MIB) + 8 (timing) = 32 bits\n\n";

  // Display complete payload
  cout << "Complete Payload (a0 to a31):\n";
  cout << payload_32bits << "\n\n";

  // ========================================================================
  // STEP 5: Display Payload Bit Map
  // ========================================================================

  cout << "Payload Bit Map:\n";
  cout << "================\n\n";

  // Display bit indices
  cout << "Bit Index: ";
  for (int i = 0; i < 32; i++) {
    cout << setw(2) << i << " ";
  }
  cout << "\n";

  // Display bit values
  cout << "Bit Value: ";
  for (int i = 0; i < 32; i++) {
    cout << " " << payload_32bits[i] << " ";
  }
  cout << "\n\n";

  // ========================================================================
  // STEP 6: Summary
  // ========================================================================

  cout << "============================================================\n";
  cout << "Summary:\n";
  cout << "============================================================\n";
  cout << "Input Parameters:\n";
  cout << "  N_ID_cell = " << N_ID_cell << "\n";
  cout << "  SFN = " << SFN << "\n";
  cout << "  Half-frame = " << half_frame << "\n";
  cout << "  SSB index = " << ssb_index << "\n";
  cout << "  SCS = " << scs << " kHz\n";
  cout << "  k_SSB = " << k_SSB << "\n";
  cout << "  L_max = " << L_max << "\n\n";

  cout << "Output:\n";
  cout << "  MIB bits (a0-a23): 24 bits\n";
  cout << "  Timing bits (a24-a31): 8 bits\n";
  cout << "  Total Payload: 32 bits\n\n";

  cout << "Final 32-bit Payload:\n";
  cout << payload_32bits << "\n";
  cout << "============================================================\n\n";

  return 0;
}
