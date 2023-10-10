
-- single line comment
-- that goes here
this shouldn't be a comment
-- ' this should
SELECT 
	cast(t.thing1 as varchar) as stuff,
	t1.thing2 as otherstuff
FROM table1 t
JOIN table2 t1 ON t.id = t1.id
GROUP BY stuff 
HAVING otherstuff != 'things and stuff
what the what?!'
order by stuff desc

/* things
Multi line comment here
stuff 
*/
