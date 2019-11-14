#!/bin/sh
#
# UCSD p-System filesystem in user space
# Copyright (C) 2006-2008 Peter Miller
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# you option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>
#

TEST_SUBJECT="ucsdpsys_text"
. test_prelude

#
# Encode the file and then decode it,
# and make sure the round trip works.
#
cat > test.in << 'fubar'
The Fence or The Ambulance
        Joseph Malins (1895)

'Twas a dangerous cliff, as they freely confessed,
 Though to walk near its crest was so pleasant;
  But over its terrible edge there had slipped
   A duke and full many a peasant.
     So the people said something would have to be done,
      But their projects did not at all tally;
       Some said, "Put a fence 'round the edge of the cliff,"
        Some, "An ambulance down in the valley."

But the cry for the ambulance carried the day,
 For it spread through the neighboring city;
  A fence may be useful or not, it is true,
   But each heart became full of pity
     For those who slipped over the dangerous cliff;
      And the dwellers in highway and alley
       Gave pounds and gave pence, not to put up a fence,
        But an ambulance down in the valley.

"For the cliff is all right, if your careful," they said,
 "And, if folks even slip and are dropping,
  It isn't the slipping that hurts them so much
   As the shock down below when they're stopping."
     So day after day, as these mishaps occurred,
      Quick forth would those rescuers sally
       To pick up the victims who fell off the cliff,
        With their ambulance down in the valley.

Then an old sage remarked: "It's a marvel to me
 That people give far more attention
  To repairing results than to stopping the cause,
   When they'd much better aim at prevention.
     Let us stop at its source all this mischief," cried he,
      "Come, neighbors and friends, let us rally;
       If the cliff we will fence, we might almost dispense
        With the ambulance down in the valley."

"Oh he's a fanatic," the others rejoined,
 "Dispense with the ambulance? Never!
  He'd dispense with all charities, too, if he could;
   No! No! We'll support them forever.
     Aren't we picking up folks just as fast as they fall?
      And shall this man dictate to us? Shall he?
       Why should people of sense stop to put up a fence,
        While the ambulance works in the valley?"

But the sensible few, who are practical too,
 Will not bear with such nonsense much longer;
  They believe that prevention is better than cure,
   And their party will soon be the stronger.
     Encourage them then, with your purse, voice, and pen,
      And while other philanthropists dally,
       They will scorn all pretense, and put up a stout fence
        On the cliff that hangs over the valley.

Better guide well the young than reclaim them when old,
 For the voice of true wisdom is calling.
  "To rescue the fallen is good, but 'tis best
   To prevent other people from falling."
     Better close up the source of temptation and crime
      Than deliver from dungeon or galley;
       Better put a strong fence 'round the top of the cliff
        Than an ambulance down in the valley.
fubar
test $? -eq 0 || no_result

ucsdpsys_text -e < test.in > test.halfway
test $? -eq 0 || fail

ucsdpsys_text -d -t < test.halfway > test.out
test $? -eq 0 || fail

diff test.in test.out
test $? -eq 0 || fail

#
# Now do the same thing, but insitu
#
cp test.in guinee.pig
test $? -eq 0 || no_result

ucsdpsys_text -e guinee.pig
test $? -eq 0 || fail

ucsdpsys_text -d -t guinee.pig
test $? -eq 0 || fail

diff test.in guinee.pig
test $? -eq 0 || fail

#
# The functionality exercised by this test worked.
# No other assertions are made.
#
pass
