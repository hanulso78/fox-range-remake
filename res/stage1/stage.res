<STAGE>
	<MUSIC>06</MUSIC>
	<ENEMY>
		<ITEM name="ENEMY1">
			<BITMAP>
				103 2 24 19 1.0
			</BITMAP>
			<WEAK>0 0 24 19</WEAK>
		</ITEM>
		<ITEM name="ENEMY2">
			<BITMAP>
				104 25 21 21 1.0
			</BITMAP>
			<WEAK>0 0 21 21</WEAK>
		</ITEM>
		<ITEM name="ENEMY3">
			<BITMAP>
				102 51 26 17 1.0
			</BITMAP>
			<WEAK>0 0 26 17</WEAK>
		</ITEM>
		<ITEM name="ENEMY4">
			<BITMAP delay="100">
				140 3 20 15 1.0
				140 19 20 15 1.0
				140 35 20 15 1.0
				140 51 20 15 1.0
			</BITMAP>
			<WEAK>0 0 20 15</WEAK>
		</ITEM>
		<ITEM name="ENEMY5">
			<BITMAP>
				177 64 32 19 1.0
			</BITMAP>
			<WEAK>0 0 32 19</WEAK>
		</ITEM>
		<ITEM name="WEAPON">
			<BITMAP>
				5 70 5 5
			</BITMAP>
		</ITEM>
		<ITEM name="BOSS">
			<BITMAP>
				0 92 100 93	1.0	<!-- normal -->
			</BITMAP>
			<WEAK>20 38 30 20</WEAK>
		</ITEM>
		<ITEM name="BOSS_DAMAGE">
			<BITMAP>
				112 92 100 93 1.0	<!-- attacked -->
			</BITMAP>
		</ITEM>
		<ITEM name="BOSS_WEAPON">
			<BITMAP>
				3 52 90 14
			</BITMAP>
		</ITEM>
	</ENEMY>
	<BACKGROUND>
		<SCROLL>
			<COLOR>0 28 137</COLOR>
			<BITMAP>
				<1>
					0 202 320 7 1.0
					0 202 320 7 1.0
				</1>
				<2>
					0 210 320 13 1.0
					0 210 320 13 1.0
				</2>
				<3>
					0 224 320 24 1.0
					0 249 320 24 1.0
					0 224 320 24 1.0
				</3>
				<4>
					0 274 320 41 1.0
					0 316 320 41 1.0
					0 274 320 41 1.0
				</4>
			</BITMAP>
			<SPEED>1 2 3 4</SPEED>
			<OVERLAP>42 36 28 10</OVERLAP>
		</SCROLL>
		<COLOR>
			<START>22 22 22</START>
			<END>200 200 22</END>
			<DIRECTION>VERTICAL</DIRECTION>
		</COLOR>
		<BITMAP>
			295 0 5 200
		</BITMAP>
	</BACKGROUND>
	<EMBLEM>
		168 4 118 48 0.7
	</EMBLEM>
</STAGE>
