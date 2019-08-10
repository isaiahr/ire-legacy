#!/usr/bin/env runhaskell
import System.Process
import GHC.IO.Handle
import System.Exit
import System.IO
import System.Environment

-- all tokens. see  lexer.h, operators.h
data Token = LParen | RParen | LSqParen | RSqParen | LCrParen | RCrParen | Integer | Char | String | Identifier
           | Term | Comma | Equals | Return | AddEq | Pipe | New | Void | Type | Colon | Dot | If
           | Plus | DoubleEquals | Less | Greater | Subtract | Mult | Ampersand | Caret deriving (Show, Eq)
           

main = putStrLn "Running fuzzer" >> run (next []) 1

-- embeds as stmt in func
embed n = [10, 10, 1, 2, 5, 11] ++ n ++ [11, 6]

run a n = (testmsg (embed a)) >> (if mod n 1000 == 0 then (putStrLn ("Tried #" ++ (show n) ++ (show (map prodToken (embed a))))) else return ()) >> (run (next a) (n+1))

next (x:xs)
    | x >= 30 = 1 : next xs
    | otherwise = (x + 1) : xs
next _ = [1]
           
-- runs a test, displaying a msg if segfaulted.
testmsg repr = do
    result <- test repr
    if result then putStrLn ("SEGV" ++ show (map prodToken repr)) else return ()

-- runs a test with a given representation
test :: [Integer] -> IO Bool
test repr = (writeTmpFile (map (addSp . getStr . prodToken) repr) >> ((openFile "/dev/null" WriteMode) >>= runprog ))


-- opens tmpfile, writes array of tokens, then closes tmpfile
writeTmpFile :: [String] -> IO ()
writeTmpFile tokens = do
    handle <- (openFile "/tmp/irefuzz" WriteMode)
    _ <- writeTokens (tokens) handle
    hClose handle
    
-- runs irec on the tmpfile. returns true if irec segfaulted.
-- runprog :: IO Bool
runprog devnull = do
    let cmd = shell "../irec /tmp/irefuzz"
    (_, _, _, ph) <- createProcess_ "runprog" cmd {std_out = UseHandle devnull, std_err = UseHandle devnull}
    let result = waitForProcess ph
    (\x -> (x == (ExitFailure (-11)))) <$> result


-- writes array of tokens to a specific handle
writeTokens :: [String] -> Handle -> IO ()
writeTokens (token:tokens) handle = ((hPutStr handle token) >> (writeTokens tokens handle))
writeTokens _ handle = hPutStr handle ""

addSp :: [Char] -> [Char]
addSp a = a ++ " "

{-
    gets a representation for a token.
    the representation doesn't matter, since we are just testing the parser.
-}
getStr :: Token -> [Char]
getStr LParen = "("
getStr RParen = ")"
getStr LSqParen = "["
getStr RSqParen = "]"
getStr LCrParen = "{"
getStr RCrParen = "}"
getStr Integer = "2"
getStr Char = "'c'"
getStr String = "\"str\""
getStr Identifier = "ident"
getStr Term = ";"
getStr Comma = ","
getStr Equals = "="
getStr Return = "return"
getStr AddEq = "+="
getStr Pipe = "|"
getStr New = "new"
getStr Void = "void"
getStr Type = "type"
getStr Colon = ":"
getStr Dot = "."
getStr If = "if"
getStr Plus = "+"
getStr DoubleEquals = "=="
getStr Less = "<"
getStr Greater = ">"
getStr Subtract = "-"
getStr Mult = "*"
getStr Ampersand = "&"
getStr Caret = "^"

{-
    produces a token given an integer.
    this is for the purpose of enumerating the tokens
-}
prodToken :: Integer -> Token
prodToken 1 = LParen
prodToken 2 = RParen
prodToken 3 = LSqParen
prodToken 4 = RSqParen
prodToken 5 = LCrParen
prodToken 6 = RCrParen
prodToken 7 = Integer
prodToken 8 = Char
prodToken 9 = String
prodToken 10 = Identifier
prodToken 11 = Term
prodToken 12 = Comma
prodToken 13 = Equals
prodToken 14 = Return
prodToken 15 = AddEq
prodToken 16 = Pipe
prodToken 17 = New
prodToken 18 = Void
prodToken 19 = Type
prodToken 20 = Colon
prodToken 21 = Dot
prodToken 22 = If
prodToken 23 = Plus
prodToken 24 = DoubleEquals
prodToken 25 = Less
prodToken 26 = Greater
prodToken 27 = Subtract
prodToken 28 = Mult
prodToken 29 = Ampersand
prodToken 30 = Caret
