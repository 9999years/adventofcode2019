module Main where

import Data.Tree
import qualified Data.Text as T

import Test.HUnit

type Orbit = (String, String)
type System = Forest String

addOrbit :: System -> Orbit -> System
addOrbit system orbit =

parseOrbit :: T.Text -> Orbit
parseOrbit line =
  let [a, b] = T.unpack <$> T.splitOn (T.singleton ')') line
   in (a, b)

parseOrbits :: String -> [Orbit]
parseOrbits str =
  let packed = T.pack str
      lns = T.lines packed
  in map parseOrbit lns

main = runTestTT tests
  where
    tests = test
      [ "parseOrbit" ~: "" ~: parseOrbit (T.pack "COM)B") ~=? ("COM", "B")
      , "parseOrbits" ~: "" ~: parseOrbits "COM)B\nB)A" ~=? [("COM", "B"), ("B", "A")]
      ]
