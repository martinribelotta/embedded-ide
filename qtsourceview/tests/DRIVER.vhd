---------------------------------------
-- driver (ESD book figure 2.3)		
--
-- two descriptions provided
----------------------------------------

library ieee;
use ieee.std_logic_1164.all;

----------------------------------------

entity Driver is
port(	x: in std_logic;
	F: out std_logic
);
end Driver;  

----------------------------------------

architecture behv1 of Driver is
begin

    process(x)
    begin
        -- compare to truth table
        if (x='1') then
            F <= '1';
        else
            F <= '0';
        end if;
    end process;

end behv1;

architecture behv2 of Driver is 
begin 

    F <= x; 

end behv2; 

------------------------------------------
